#pragma once

#include "stm32h7xx_hal.h"

#define BOOT1_PIN GPIO_PIN_2
#define BOOT1_PORT GPIOB

#define LED_PIN GPIO_PIN_7
#define LED_PORT GPIOG
#define LED_ACTIVEHIGH false

void SystemClock_Config(void);