#include "main.h"
#include "variant.h"

#include "stm32f4xx_hal.h"


int main(void)
{

    HAL_Init();
    SystemClock_Config();
    while (1)
    {
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}