// win_usb.cpp
#include "nusb.h"

#include <tchar.h>
#include <strsafe.h>
#include <windows.h>

#include <winusb.h>
#include <usb100.h>
#include <setupapi.h>
#include <thread>
#include <condition_variable>
#include <mutex>

#pragma comment(lib,"setupapi.lib")
#pragma comment(lib,"winusb.lib")

class semaphore {
public:
  semaphore(int init_count = 1)
    : count_(init_count) { }

  void lock()
  {
    std::unique_lock<std::mutex> lk(m_);
    cv_.wait(lk, [=]{ return 0 < count_; });
    --count_;
  }
  bool try_lock()
  {
    std::lock_guard<std::mutex> lk(m_);
    if (0 < count_) {
      --count_;
      return true;
    }
    return false;
  }
  void unlock()
  {
    std::lock_guard<std::mutex> lk(m_);
    if (count_ < 1) {
      ++count_;
      cv_.notify_one();
    }
  }

private:
  int count_;
  std::mutex m_;
  std::condition_variable cv_;
};

class NakedUsbImpl :
  public NakedUsb
{
public:
  NakedUsbImpl();
  ~NakedUsbImpl();
  bool Open(LPGUID interface_guid);
  void Close();
  inline const std::vector<Endpoint>& Endpoints()const{ return ends_; }
  bool onEasyRead(std::function<void(uint8_t *buf, size_t len)> cb);
  void Poll();
  int ReadSync(const Endpoint *ep, uint8_t *buf, int len);
  void WriteSync(const Endpoint *ep, const uint8_t *buf, int len);

  int Control(uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint8_t *data, size_t len);
private:
  struct Async{
    Endpoint ep;
    std::vector<std::vector<uint8_t> > buf;
    std::function<void(uint8_t *buf, size_t len)> cb;
    ULONG reads;
    std::thread *thread;
    semaphore *sem;
    bool exit;
  };
  bool GetDevicePath(LPGUID interface_guid, LPTSTR device_path, size_t buf_len);
  LPGUID interface_guid_;
  HANDLE device_handle_;
  WINUSB_INTERFACE_HANDLE win_usb_handle_;
  std::vector<Endpoint> ends_;
  std::vector<Async> event_;
  int device_speed_;
};

NakedUsb* NakedUsb::Create(){
  return new NakedUsbImpl;
}

bool NakedUsbImpl::GetDevicePath(LPGUID interface_guid, LPTSTR device_path, size_t buf_len)
{
  HDEVINFO device_info;
  SP_DEVICE_INTERFACE_DATA interface_data;

  device_info = SetupDiGetClassDevs(interface_guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
  if (SUCCEEDED(SetupDiEnumDeviceInterfaces(device_info, NULL, interface_guid, 0, &interface_data)))
  {
    ULONG required_length = 0;
    if (SUCCEEDED(SetupDiGetDeviceInterfaceDetail(device_info, &interface_data, NULL, 0, &required_length, NULL)) && required_length)
    {
      auto detail_data = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LMEM_FIXED, required_length);
      detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
      if (SUCCEEDED(SetupDiGetDeviceInterfaceDetail(device_info, &interface_data, detail_data,
        required_length, &required_length, NULL)))
      {

        StringCchCopy(device_path, buf_len, detail_data->DevicePath);

        LocalFree(detail_data);
        return true;
      }

      LocalFree(detail_data);

    }
    SetupDiDestroyDeviceInfoList(device_info);
  }
  return false;
}

NakedUsbImpl::NakedUsbImpl()
  :device_handle_(nullptr)
  , win_usb_handle_(nullptr)
{

}

bool NakedUsbImpl::Open(LPGUID interface_guid)
{
  char device_path[_MAX_PATH + 1];

  GetDevicePath(interface_guid, (LPTSTR)device_path, sizeof(device_path) / sizeof(device_path[0]));

  if (device_handle_ = CreateFile((LPTSTR)device_path, GENERIC_WRITE | GENERIC_READ,
    FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL))
  {

    if (TRUE == WinUsb_Initialize(device_handle_, &win_usb_handle_))
    {
      ULONG length = sizeof(unsigned char);
      WinUsb_QueryDeviceInformation(win_usb_handle_, DEVICE_SPEED, &length, &device_speed_);

      USB_INTERFACE_DESCRIPTOR iface_descriptor;
      WinUsb_QueryInterfaceSettings(win_usb_handle_, 0, &iface_descriptor);

      WINUSB_PIPE_INFORMATION pipe_info;
      for (int i = 0; i < iface_descriptor.bNumEndpoints; i++)
      {
        WinUsb_QueryPipe(win_usb_handle_, 0, (unsigned char)i, &pipe_info);
        Endpoint ep;
        ep.address = pipe_info.PipeId;
        ep.input = (pipe_info.PipeId & 0x80) ? true : false;
        ep.interval = pipe_info.Interval;
        ep.max_size = pipe_info.MaximumPacketSize;

        if (USB_ENDPOINT_DIRECTION_IN(pipe_info.PipeId))
        {
          BOOL tr = TRUE;
          WinUsb_SetPipePolicy(win_usb_handle_, pipe_info.PipeId, AUTO_CLEAR_STALL, 4, &tr);
          WinUsb_SetPipePolicy(win_usb_handle_, pipe_info.PipeId, RESET_PIPE_ON_RESUME, 4, &tr);
          WinUsb_SetPipePolicy(win_usb_handle_, pipe_info.PipeId, AUTO_FLUSH, 4, &tr);
          ULONG to = 1500;
          WinUsb_SetPipePolicy(win_usb_handle_, pipe_info.PipeId, PIPE_TRANSFER_TIMEOUT, 4, &to);
        }

        if (UsbdPipeTypeInterrupt == pipe_info.PipeType){
          ep.type = EndpointType::Interrupt;
        }
        else if (UsbdPipeTypeBulk == pipe_info.PipeType){
          ep.type = EndpointType::Bulk;
        }
        ends_.push_back(ep);
      }
      return true;
    }
  }
  Close();
  return false;
}

void NakedUsbImpl::Close()
{
  device_handle_ && CloseHandle(device_handle_);
  win_usb_handle_ && WinUsb_Free(win_usb_handle_);
  for (auto e : event_){
    e.exit = true;
    e.thread->join();
    delete e.thread;
    delete e.sem;
  }
}

NakedUsbImpl::~NakedUsbImpl()
{
  Close();
}

bool NakedUsbImpl::onEasyRead(std::function<void(uint8_t *buf, size_t len)> cb){
  for (auto e : ends_){
    if (e.input){
      volatile bool wait = true;
      Async a;
      a.ep = e;
      a.cb = cb;
      a.reads = 0;
      a.exit = false;
      a.sem = new semaphore();
      a.thread = new std::thread([this, e, &wait](){
        while (wait)std::this_thread::sleep_for(std::chrono::microseconds(1));
        for (auto &a : event_){
          if (a.ep.address == e.address){
            while (!a.exit){
              std::vector<uint8_t> buf;
              buf.resize(a.ep.max_size);
              WinUsb_ReadPipe(win_usb_handle_, e.address, &*buf.begin(), buf.size(), &a.reads, NULL);
              buf.resize(a.reads);
              a.sem->lock();
              a.buf.push_back(buf);
              a.sem->unlock();
            }
            return;
          }
        }
      });

      event_.push_back(a);
      wait = false;//TODO: dirty
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      return true;
    }
  }
  return false;
}

void NakedUsbImpl::Poll(){
  for (auto &a : event_){
    std::vector<std::vector<uint8_t> > buf;
    a.sem->lock();
    buf = a.buf;
    a.buf.clear();
    a.sem->unlock();
    for (auto b : buf){
      if (0 < b.size())
        a.cb(&*b.begin(), b.size());
    }
  }
}

int NakedUsbImpl::ReadSync(const Endpoint *ep, uint8_t *buf, int len)
{
  int bytes_read;
  WinUsb_ReadPipe(win_usb_handle_, ep->address, buf, len, (PULONG)&bytes_read, NULL);
  return bytes_read;
}

void NakedUsbImpl::WriteSync(const Endpoint *ep, const uint8_t *buf, int len)
{
  int bytes;
  WinUsb_WritePipe(win_usb_handle_, ep->address, const_cast<PUCHAR>(buf), len, (PULONG)&bytes, NULL);
  WinUsb_FlushPipe(win_usb_handle_, ep->address);
}

int NakedUsbImpl::Control(uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint8_t *data, size_t len)
{
  WINUSB_SETUP_PACKET p;
  p.RequestType = request_type;
  p.Request = request;
  p.Value = value;
  p.Index = index;
  p.Length = (USHORT)len;
  ULONG transfered = 0;
  WinUsb_ControlTransfer(win_usb_handle_, p, data, len, &transfered, NULL);
  if (len != transfered){
    return -1;
  }
  return transfered;
}
