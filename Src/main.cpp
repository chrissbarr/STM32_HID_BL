#include "main.h"
#include "variant.h"

// #define SD_CS_Pin GPIO_PIN_4
// #define SD_CS_GPIO_Port GPIOE
// #define SD_SPI_HANDLE hspi4

#include "flash_wrapper.h"
#include "rtc_wrapper.h"
#include "usb_wrapper.h"
#include "fatfs.h"
#include "sd_wrapper.h"

#include <array>
#include <cstring>

constexpr uint16_t HID_RX_SIZE = 64;
uint8_t USB_RX_Buffer[HID_RX_SIZE];
volatile uint8_t new_data_is_received = false;

constexpr int bootloaderPreBootWait = 5000;



/* Buffer FW received from USB is loaded into */
std::array<uint8_t, 1024> flashPageData;
/* Current write position in pageData */

uint16_t pageOffset = 0;

/* Number of flash sectors dedicated to bootloader */
constexpr uint32_t bootloaderFlashSectors = 1;
/* Space taken by bootloader / offset user app starts at (Bytes) */
constexpr uint32_t appFlashOffset = FlashSectors[bootloaderFlashSectors].start - FlashSectors[0].start;
/* Address user app starts at */
constexpr uint32_t appFlashStart = FlashSectors[0].start + appFlashOffset;
/* Current write position in Flash (in units of flashPageSize) */
uint32_t currentFlashPage = 0;

bool anyCommandsReceived = false;

static uint8_t HIDCommandSig[7] = {'B', 'T', 'L', 'D', 'C', 'M', 'D'};

enum class HIDCommand : uint8_t {
    ResetPages = 0x00,
    ResetMCU = 0x01,
    AckDataRecv = 0x02,
    LEDFlash = 0x03,
};

/* Internal function declarations */
void GPIO_Init();
void Set_LED(bool on);
void Start_User_Application();
bool Boot_Pin_Set();

int main(void) {
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();



    #ifdef SD_ENABLED
        Initialise_SD();
    #endif

    bool startBootloader = false;
    if (RTC_Magic_Value_Set()) {
        RTC_Clear_Magic_Value();
        startBootloader = true;
    }

    if (Boot_Pin_Set()) { startBootloader = true; }

    if (!startBootloader && bootloaderPreBootWait == 0) { Start_User_Application(); }

    for (int i = 0; i < 5; i++) {
        Set_LED(true);
        HAL_Delay(50);
        Set_LED(false);
        HAL_Delay(50);
    }

    USB_Init();

    while (1) {
        if (new_data_is_received == 1) {
            new_data_is_received = 0;
            anyCommandsReceived = true;

            /* If data received is command, process command */
            if (memcmp(USB_RX_Buffer, HIDCommandSig, sizeof(HIDCommandSig)) == 0) {
                HIDCommand cmd = static_cast<HIDCommand>(USB_RX_Buffer[7]);
                switch (cmd) {
                case HIDCommand::ResetPages: {

                    /* Reset buffers and write position so that we are writing from start */
                    currentFlashPage = 0;
                    pageOffset = 0;

                    break;
                }
                case HIDCommand::ResetMCU: {
                    /* Flush any leftover data in the buffer */
                    if (pageOffset > 0) { Write_Flash_Sector(flashPageData, FlashSectors, currentFlashPage); }
                    HAL_Delay(100);
                    HAL_NVIC_SystemReset();
                    break;
                }
                case HIDCommand::LEDFlash: {
                    Set_LED(true);
                    HAL_Delay(500);
                    Set_LED(false);
                    break;
                }
                default: {
                    Error_Handler();
                }
                }
            } else {
                /* If not a command, we have received FW data*/
                /* Copy the FW data into the buffer */
                memcpy(flashPageData.data() + pageOffset, USB_RX_Buffer, HID_RX_SIZE);
                pageOffset += HID_RX_SIZE;

                /* If buffer is full, write FW out to flash */
                if (pageOffset == flashPageData.size()) {
                    Set_LED(true);
                    Write_Flash_Sector(flashPageData, FlashSectors, currentFlashPage);
                    Set_LED(false);
                    currentFlashPage++;

                    /* Reset buffer */
                    pageOffset = 0;

                    uint8_t cmdSend[8];
                    memcpy(cmdSend, HIDCommandSig, sizeof(HIDCommandSig));
                    cmdSend[7] = static_cast<uint8_t>(HIDCommand::AckDataRecv);
                    USB_HID_SendReport(cmdSend, 8);
                }
            }
        }

        if (!startBootloader && !anyCommandsReceived && HAL_GetTick() > bootloaderPreBootWait) {
            Set_LED(true);
            HAL_Delay(50);
            Set_LED(false);
            HAL_Delay(50);
            Start_User_Application();
        }
    }
}

void Start_User_Application() {
    HAL_DeInit();

    typedef void (*pFunction)(void);
    pFunction Jump_To_Application;
    uint32_t JumpAddress;

    JumpAddress = *(__IO uint32_t*)(appFlashStart + 4);
    Jump_To_Application = (pFunction)JumpAddress;
    __set_MSP(*(uint32_t*)(appFlashStart));
    Jump_To_Application();
}

void GPIO_Init() {
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Configure LED */
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

    /* Configure BOOT1 Input */
    #ifdef BOOT1_PIN
        GPIO_InitStruct.Pin = BOOT1_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        /* Configure pull up/down resistor to counter button active state */
        if (BOOT1_ACTIVEHIGH) {
            GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        } else {
            GPIO_InitStruct.Pull = GPIO_PULLUP;
        }
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(BOOT1_PORT, &GPIO_InitStruct);
    #endif

    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = SD_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SD_CS_GPIO_Port, &GPIO_InitStruct);
}


void Set_LED(bool on) {
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, on == LED_ACTIVEHIGH ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool Boot_Pin_Set() {
    bool set = false;
#ifdef BOOT1_PIN
    set = HAL_GPIO_ReadPin(BOOT1_PORT, BOOT1_PIN) == BOOT1_ACTIVEHIGH;
#endif
    return set;
}

extern "C" void Error_Handler(void) {
    __disable_irq();
    while (1) {}
}