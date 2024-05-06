#include "usb_wrapper.h"
#include "usb_device.h"
#include "usbd_custom_hid_if.h"

void USB_Init()
{
    MX_USB_DEVICE_Init();
}

void USB_HID_SendReport(uint8_t *data, uint8_t size)
{
    USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, data, size);
}
