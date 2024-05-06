#pragma once

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

void USB_Init();

void USB_HID_SendReport(uint8_t* data, uint8_t size);

#ifdef __cplusplus
}
#endif

