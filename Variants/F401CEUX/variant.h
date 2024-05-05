#pragma once

/* Project Includes */
#include "flash.h"

/* STM32 HAL */
#include "stm32f4xx_hal.h"

/* C++ Standard Library */
#include <array>

extern "C" void SystemClock_Config(void);

#define BOOT1_PIN GPIO_PIN_0
#define BOOT1_PORT GPIOA

#define LED_PIN GPIO_PIN_13
#define LED_PORT GPIOC
#define LED_ACTIVEHIGH true

constexpr std::array<FlashSector, 8> FlashSectors{{
    {0x08000000, 0x4000},
    {0x08004000, 0x4000},
    {0x08008000, 0x4000},
    {0x0800C000, 0x4000},
    {0x08010000, 0x10000},
    {0x08020000, 0x20000},
    {0x08040000, 0x20000},
    {0x08060000, 0x20000},
}};