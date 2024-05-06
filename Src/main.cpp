#include "main.h"
#include "variant.h"

#include "usb_device.h"

#include <array>

constexpr uint16_t HID_RX_SIZE = 64;
uint8_t USB_RX_Buffer[HID_RX_SIZE];
volatile uint8_t new_data_is_received = false;

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

static uint8_t HIDCommandSig[7] = {'B','T','L','D','C','M','D'};

enum class HIDCommand : uint8_t {
    ResetPages = 0x00,
    ResetMCU = 0x01,
    AckDataRecv = 0x02,
    LEDFlash = 0x03,
};

/* Internal function declarations */
void GPIO_Init();
void write_flash_sector(uint32_t currentPage);
void set_LED(bool on);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    USB_Init();

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

                        /* Reset buffers and write position so that we are writing from start */
                        currentFlashPage = 0;
                        pageOffset = 0;

                        break;
                    }
                    case HIDCommand::ResetMCU: {
                        /* Flush any leftover data in the buffer */
                        if (pageOffset > 0) {
                            write_flash_sector(currentFlashPage);
                        }
                        HAL_Delay(100);
                        HAL_NVIC_SystemReset();
                        break;
                    }
                    case HIDCommand::LEDFlash: {
                        set_LED(true);
                        HAL_Delay(500);
                        set_LED(false);
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
                    write_flash_sector(currentFlashPage);
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

void write_flash_sector(uint32_t currentPage)
{
    /* Address in flash memory that the page we are writing will start from */
    const uint32_t pageAddress = appFlashStart + (currentPage * flashPageData.size());

    set_LED(true);

    HAL_FLASH_Unlock();

    /* Check if pageAddress is start of new flash sector */
    int eraseSector = -1;
    for (int i = 0; i < static_cast<int>(FlashSectors.size()); i++) {
        if (FlashSectors[i].start == pageAddress) {
            eraseSector = i;
            break;
        }
    }

    /* If page is in a new flash sector, need to erase flash sector before writing */
    if (eraseSector != -1)
    {
        FLASH_EraseInitTypeDef EraseInit{};
        EraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
        EraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        EraseInit.Sector = eraseSector;
        EraseInit.NbSectors = 1;
        uint32_t SectorError;
        HAL_FLASHEx_Erase(&EraseInit, &SectorError);
    }

    /* Write page to flash one word at a time */
    for (std::size_t i = 0; i < flashPageData.size(); i += 4)
    {
        uint32_t data = 0;
        data = flashPageData[i + 3];
        data <<= 8;
        data += flashPageData[i + 2];
        data <<= 8;
        data += flashPageData[i + 1];
        data <<= 8;
        data += flashPageData[i];
        uint32_t writeAddr = pageAddress + i;

        if (writeAddr >= FlashSectors.back().start + FlashSectors.back().size) { break; }

        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, pageAddress + i, data);
    }

    HAL_FLASH_Lock();
    set_LED(false);
}

void set_LED(bool on)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, on == LED_ACTIVEHIGH ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

extern "C" void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}