#include "main.h"
#include "variant.h"

#include "usb_device.h"

#define HID_RX_SIZE 64
uint8_t USB_RX_Buffer[HID_RX_SIZE];
volatile uint8_t new_data_is_received = false;

static uint8_t HIDCommandSig[7] = {'B','T','L','D','C','M','D'};

enum class HIDCommand : uint8_t {
    ResetPages = 0x00,
    ResetMCU = 0x01,
    LEDFlash = 0x02,
};


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
    MX_USB_DEVICE_Init();

    set_LED(true);
    HAL_Delay(500);
    set_LED(false);
    HAL_Delay(500);

    while (1)
    {
        if (new_data_is_received == 1) {
            new_data_is_received = 0;

            /* If data received is command, process command */
            if (memcmp(USB_RX_Buffer, HIDCommandSig, sizeof(HIDCommandSig)) == 0)
            {
                HIDCommand cmd = static_cast<HIDCommand>(USB_RX_Buffer[7]);
                switch(cmd) {
                    case HIDCommand::ResetPages: {

                        break;
                    }
                    case HIDCommand::ResetMCU: {

                        break;
                    }
                    case HIDCommand::LEDFlash: {
                        set_LED(true);
                        HAL_Delay(500);
                        set_LED(false);
                        break;
                    }
                }
            }
        }
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
}

extern "C" void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}