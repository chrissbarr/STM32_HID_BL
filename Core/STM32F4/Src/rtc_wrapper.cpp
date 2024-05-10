#include "rtc_wrapper.h"

#include "stm32f4xx_ll_rtc.h"
#include "stm32f4xx_ll_pwr.h"

constexpr int MagicValue = 0x424C;
constexpr uint32_t MagicValueRTCRegister = LL_RTC_BKP_DR4;

bool RTC_Magic_Value_Set()
{
    int regValue = LL_RTC_BAK_GetRegister(RTC, MagicValueRTCRegister);
    return (regValue == MagicValue);
}

void RTC_Clear_Magic_Value()
{
    LL_PWR_EnableBkUpAccess();
    LL_RTC_BAK_SetRegister(RTC, MagicValueRTCRegister, 0);
    LL_PWR_DisableBkUpAccess();
}