#pragma once

/* Project Includes */
#include "flash.h"

/* STM32 HAL */
#include "stm32h7xx_hal.h"

/* C++ Standard Library */
#include <array>

extern "C" void SystemClock_Config(void);

#define BOOT1_PIN GPIO_PIN_2
#define BOOT1_PORT GPIOB
#define BOOT1_ACTIVEHIGH true

#define LED_PIN GPIO_PIN_5
#define LED_PORT GPIOF
#define LED_ACTIVEHIGH true

#define SD_ENABLED
#define SD_BIN_FILENAME "volumic.bin"

constexpr std::array<FlashSector, 8> FlashSectors{{
    {0x08000000, 0x20000},
    {0x08020000, 0x20000},
    {0x08040000, 0x20000},
    {0x08060000, 0x20000},
    {0x08080000, 0x20000},
    {0x080A0000, 0x20000},
    {0x080C0000, 0x20000},
    {0x080E0000, 0x20000},
}};