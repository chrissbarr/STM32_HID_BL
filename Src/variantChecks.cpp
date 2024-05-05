#include "variant.h"

/* This file provides a set of compile-time assertions that test the defined variant. */

static_assert(FlashSectors[0].start == 0x08000000, "ERROR");

/* Code assumes flash is laid out contiguously. Ensure this is the case. */
constexpr bool FlashSectorsContiguous() {
    for (std::size_t i = 1; i < FlashSectors.size(); i++) {
        if (FlashSectors[i - 1].start + FlashSectors[i - 1].size != FlashSectors[i].start) {
            return false;
        }
    }
    return true;
}
static_assert(
    FlashSectorsContiguous(),
    "ERROR - Flash sectors for configured variant are not contiguous / do not align. Check FlashSectors definition.");

/* Code assumes flash sectors are multiples of 1024B. Ensure this is the case. */
constexpr bool FlashSectorsSizes() {
    for (std::size_t i = 0; i < FlashSectors.size(); i++) {
        if (FlashSectors[i].size % 1024 != 0) {
            return false;
        }
    }
    return true;
}
static_assert(
    FlashSectorsSizes(),
    "ERROR - Flash sectors for configured variant are not multiples of 1024B. Check FlashSectors definition.");