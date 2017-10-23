#ifndef _IUSB_H_
#define _IUSB_H_

#include <stdint.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

#include <winusb.h>
#include <usb100.h>
#include <setupapi.h>


BOOL GetDevicePath(LPGUID interface_guid, LPTSTR device_path, size_t buf_len);
HANDLE OpenDevice(LPGUID interface_guid, BOOL sync);

class Iusb {
public:
    Iusb(LPGUID interface_guid);
    ~Iusb();
    int Read(uint8_t *buf,size_t len);
    int ReadInterrupt(uint8_t *buf,size_t len);
    int Control(uint8_t request_type,uint8_t request,uint16_t value,uint16_t index,uint8_t *data,size_t len);
    void WriteInterrupt(const uint8_t *buf,size_t len);

private:
    LPGUID interface_guid_;
    HANDLE device_handle_;
    WINUSB_INTERFACE_HANDLE win_usb_handle_;
    unsigned char bulk_in_pipe_;
    unsigned char bulk_out_pipe_;
    unsigned char interrupt_in_;
    unsigned char interrupt_out_;
    int device_speed_;
};

#endif