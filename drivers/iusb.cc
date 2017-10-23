// win_usb.cpp
#include "iusb.h"

#pragma comment(lib,"setupapi.lib")
#pragma comment(lib,"winusb.lib")

#include <assert.h>
BOOL GetDevicePath(LPGUID interface_guid, LPTSTR device_path, size_t buf_len)
{
    BOOL result = FALSE;
    HDEVINFO device_info;
    SP_DEVICE_INTERFACE_DATA interface_data;
    PSP_DEVICE_INTERFACE_DETAIL_DATA detail_data = NULL;
    ULONG length;
    ULONG required_length = 0;
    HRESULT hr;

    device_info = SetupDiGetClassDevs(interface_guid, NULL, NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    result = SetupDiEnumDeviceInterfaces(device_info, NULL, interface_guid, 0,
        &interface_data);
    SetupDiGetDeviceInterfaceDetail(device_info, &interface_data, NULL, 0,
        &required_length, NULL);
    detail_data = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LMEM_FIXED,
        required_length);

    if (detail_data == NULL) {
        SetupDiDestroyDeviceInfoList(device_info);
        return FALSE;
    }

    detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    length = required_length;

    result = SetupDiGetDeviceInterfaceDetail(device_info, &interface_data,
        detail_data, length,
        &required_length, NULL);
    if (result == FALSE) {
        LocalFree(detail_data);
        return FALSE;
    }

    hr = StringCchCopy(device_path, buf_len, detail_data->DevicePath);

    if (FAILED(hr)) {
        SetupDiDestroyDeviceInfoList(device_info);
        LocalFree(detail_data);
    }

    LocalFree(detail_data);

    return result;
}

HANDLE OpenDevice(LPGUID interface_guid, bool sync)
{
    HANDLE device_handle = NULL;
    char device_path[_MAX_PATH + 1];

    BOOL retval = GetDevicePath(interface_guid, (LPTSTR)device_path,
        sizeof(device_path) / sizeof(device_path[0]));
    device_handle = CreateFile((LPTSTR)device_path,
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_WRITE | FILE_SHARE_READ, NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL);
    return device_handle;
}

Iusb::Iusb(LPGUID interface_guid)
{
    BOOL result;
    USB_INTERFACE_DESCRIPTOR iface_descriptor;
    WINUSB_PIPE_INFORMATION pipe_info;
    ULONG length;

    device_handle_ = OpenDevice(interface_guid, true);
    result = WinUsb_Initialize(device_handle_, &win_usb_handle_);

    if (result) {
        length = sizeof(unsigned char);
        result = WinUsb_QueryDeviceInformation(win_usb_handle_, DEVICE_SPEED,
            &length, &device_speed_);
    }

    if (result) {
        result = WinUsb_QueryInterfaceSettings(win_usb_handle_, 0,
            &iface_descriptor);
    }

    if (result) {
        for (int i = 0; i < iface_descriptor.bNumEndpoints; i++) {
            result = WinUsb_QueryPipe(win_usb_handle_, 0, (unsigned char)i,&pipe_info);
            if(UsbdPipeTypeInterrupt==pipe_info.PipeType){
                if(USB_ENDPOINT_DIRECTION_IN(pipe_info.PipeId))
                {
                    interrupt_in_ = pipe_info.PipeId;
                    BOOL tr=TRUE;
                    WinUsb_SetPipePolicy(win_usb_handle_,interrupt_in_,AUTO_CLEAR_STALL,4,&tr);
                    WinUsb_SetPipePolicy(win_usb_handle_,interrupt_in_,ALLOW_PARTIAL_READS,4,&tr);
                    WinUsb_SetPipePolicy(win_usb_handle_,interrupt_in_,AUTO_FLUSH,4,&tr);
                    ULONG to=1000;
                    WinUsb_SetPipePolicy(win_usb_handle_,interrupt_in_,PIPE_TRANSFER_TIMEOUT,4,&to);
                }
                else
                    interrupt_out_ = pipe_info.PipeId;
            }else if(UsbdPipeTypeBulk==pipe_info.PipeType){
                if(USB_ENDPOINT_DIRECTION_IN(pipe_info.PipeId))
                {
                    bulk_in_pipe_ = pipe_info.PipeId;
                    BOOL tr=TRUE;
                    WinUsb_SetPipePolicy(win_usb_handle_,bulk_in_pipe_,AUTO_CLEAR_STALL,4,&tr);
                    WinUsb_SetPipePolicy(win_usb_handle_,bulk_in_pipe_,RESET_PIPE_ON_RESUME,4,&tr);
                    WinUsb_SetPipePolicy(win_usb_handle_,bulk_in_pipe_,AUTO_FLUSH,4,&tr);
                    ULONG to=1500;
                    WinUsb_SetPipePolicy(win_usb_handle_,bulk_in_pipe_,PIPE_TRANSFER_TIMEOUT,4,&to);
                }
                else
                {
                    bulk_out_pipe_ = pipe_info.PipeId;
                }
            }else{
                break;
            }
        }
    }
}

Iusb::~Iusb()
{
    WinUsb_Free(win_usb_handle_);
    CloseHandle(device_handle_);
}

int Iusb::Read(uint8_t *buf,size_t len)
{
    int bytes_read;
    WinUsb_ReadPipe(win_usb_handle_, bulk_in_pipe_,buf,len,(PULONG)&bytes_read, NULL);
    return bytes_read;
}

BOOL Iusb::ReadInterrupt(uint8_t *buf,size_t len)
{
    int bytes_read;
    
    WinUsb_ReadPipe(win_usb_handle_, interrupt_in_,buf,len,(PULONG)&bytes_read, NULL);
    return bytes_read;
}

void Iusb::WriteInterrupt(const uint8_t *buf,size_t len)
{
    int bytes;
    WinUsb_WritePipe(win_usb_handle_,interrupt_out_,const_cast<PUCHAR>(buf),len,(PULONG)&bytes,NULL);
    WinUsb_FlushPipe(win_usb_handle_,interrupt_out_);
}

int Iusb::Control(uint8_t request_type,uint8_t request,uint16_t value,uint16_t index,uint8_t *data,size_t len)
{
    WINUSB_SETUP_PACKET p;
    p.RequestType=request_type;
    p.Request=request;
    p.Value=value;
    p.Index=index;
    p.Length=len;
    ULONG transfered=0;
    WinUsb_ControlTransfer(win_usb_handle_,p,data,len,&transfered,NULL);
    if(len!=transfered){
        return -1;
    }
    return transfered;
}
