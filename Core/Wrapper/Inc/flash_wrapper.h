#pragma once

#include "flash.h"

#include <span>

void Write_Flash_Sector(std::span<uint8_t> pageData, std::span<const FlashSector> flashSectors, uint32_t currentPage);
