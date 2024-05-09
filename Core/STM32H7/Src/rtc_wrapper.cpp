#include "rtc_wrapper.h"

#include "stm32h7xx_ll_rtc.h"
#include "stm32h7xx_ll_pwr.h"

constexpr int MagicValue = 0x424C;
constexpr uint32_t MagicValueRTCRegister = LL_RTC_BKP_DR4;

bool RTCMagicValueSet()
{
    int regValue = LL_RTC_BAK_GetRegister(RTC, MagicValueRTCRegister);
    return (regValue == MagicValue);
}

void RTCClearMagicValue()
{
    LL_PWR_EnableBkUpAccess();
    LL_RTC_BAK_SetRegister(RTC, MagicValueRTCRegister, 0);
    LL_PWR_DisableBkUpAccess();
}