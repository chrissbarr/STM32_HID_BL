#pragma once

#include "stm32f4xx_hal.h"

#define BOOT1_PIN GPIO_PIN_0
#define BOOT1_PORT GPIOA

#define LED_PIN GPIO_PIN_13
#define LED_PORT GPIOC
#define LED_ACTIVEHIGH true

void SystemClock_Config(void);