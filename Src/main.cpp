#include "main.h"
#include "variant.h"

void GPIO_Init();

void set_LED(bool on)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, on == LED_ACTIVEHIGH ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

int main(void)
{

    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    set_LED(true);
    HAL_Delay(500);
    set_LED(false);
    HAL_Delay(500);

    while (1)
    {
        set_LED(true);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(100);
        set_LED(false);
         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(100);
    }
}

void GPIO_Init()
{

    /* Configure LED */
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}