#pragma once

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>

#define SD_CS_Pin GPIO_PIN_4
#define SD_CS_GPIO_Port GPIOE
#define SD_SPI_HANDLE hspi

void Error_Handler(void);
extern void SystemClock_Config(void);

void Set_LED(bool on);

#ifdef __cplusplus
}
#endif
