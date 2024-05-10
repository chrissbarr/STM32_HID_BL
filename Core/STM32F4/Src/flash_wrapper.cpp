#include "flash_wrapper.h"

#include "stm32f4xx_hal.h"

void Write_Flash_Sector(std::span<uint8_t> flashPageData, std::span<const FlashSector> flashSectors, uint32_t currentPage)
{
    uint32_t appFlashStart = flashSectors[1].start;
    /* Address in flash memory that the page we are writing will start from */
    const uint32_t pageAddress = appFlashStart + (currentPage * flashPageData.size());

    HAL_FLASH_Unlock();

    /* Check if pageAddress is start of new flash sector */
    int eraseSector = -1;
    for (int i = 0; i < static_cast<int>(flashSectors.size()); i++)
    {
        if (flashSectors[i].start == pageAddress)
        {
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

        if (writeAddr >= flashSectors.back().start + flashSectors.back().size)
        {
            break;
        }

        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, pageAddress + i, data);
    }

    HAL_FLASH_Lock();
}
