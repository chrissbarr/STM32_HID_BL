#pragma once

/* C++ Standard Library */
#include <cstdint>

struct FlashSector {
    /* Start address of flash sector */
    uint32_t start;
    /* Size of flash sector (Bytes) */
    uint32_t size;
};