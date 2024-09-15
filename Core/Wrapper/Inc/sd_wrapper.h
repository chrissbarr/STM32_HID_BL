#pragma once

#include "flash.h"

#include <span>

bool attempt_install_from_Sd(std::span<const FlashSector> flashSectors);