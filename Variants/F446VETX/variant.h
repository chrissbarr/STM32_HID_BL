#pragma once

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define BOOT1_PIN GPIO_PIN_2
#define BOOT1_PORT GPIOB

#define LED_PIN GPIO_PIN_14
#define LED_PORT GPIOB
#define LED_ACTIVEHIGH true

void SystemClock_Config(void);

#ifdef __cplusplus
}
#endif