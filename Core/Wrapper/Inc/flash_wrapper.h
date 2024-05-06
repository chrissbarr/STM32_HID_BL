#pragma once

#include "flash.h"

#include <span>

void write_flash_sector(std::span<uint8_t> pageData, std::span<const FlashSector> flashSectors, uint32_t currentPage);