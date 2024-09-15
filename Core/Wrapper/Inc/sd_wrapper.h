#pragma once

#include "flash.h"

#include <span>

bool Initialise_SD(std::span<const FlashSector> flashSectors);