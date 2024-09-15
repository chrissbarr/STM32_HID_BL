#include "sd_wrapper.h"
#include "flash_wrapper.h"
#include "main.h"
#include "fatfs.h"

#include <array>
#include <cstdint>

#include "stm32h7xx_hal.h"

SPI_HandleTypeDef hspi;

void MX_SPI4_Init(void)
{
  hspi.Instance = SPI4;
  hspi.Init.Mode = SPI_MODE_MASTER;
  hspi.Init.Direction = SPI_DIRECTION_2LINES;
  hspi.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi.Init.NSS = SPI_NSS_SOFT;
  hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi.Init.CRCPolynomial = 0x0;
  hspi.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI4_Init 2 */

  /* USER CODE END SPI4_Init 2 */

}

uint32_t crc32iso_hdlc_bit(uint32_t crc, uint8_t const *mem, size_t len) {
    unsigned char const *data = mem;
    if (data == NULL)
        return 0;
    crc = ~crc;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (unsigned k = 0; k < 8; k++) {
            crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        }
    }
    crc = ~crc;
    return crc;
}

uint32_t uint32_from_uint8(uint8_t* buffer)
{
    uint32_t val = 0;
    val |= buffer[0] << 0;
    val |= buffer[1] << 8;
    val |= buffer[2] << 16;
    val |= buffer[3] << 24;
    return val;
}

bool Initialise_SD(std::span<const FlashSector> flashSectors) {

    // 1 - Initialise SD card access (if available)
    //       If yes, continue. If not abort.
    // 2 - Check if a valid FW update file is available.
    //       Valid means:
    //         Signed as expected
    //         CRC matches
    //         Size will fit in available flash space
    //       If yes, continue. If not abort.
    // 3 - Dump the currently flashed FW to a backup file on the SD card.

    MX_SPI4_Init();
    MX_FATFS_Init();

    FATFS FatFs;    //Fatfs handle
    FRESULT fres;

    // Open the file system
    fres = f_mount(&FatFs, "", 1 /* 1 = mount now */);
    if (fres != FR_OK) {
        return false;
    }

    // Check for a FW update file
    static const char filename[] = "sd_fw.bin"; // sd_fw_nosig
    FIL file;
    fres = f_open(&file, filename, FA_READ);
    if (fres != FR_OK) {
        return false;
    }

    // Read signature from first 4 bytes
    std::array<uint8_t, 4> sigBuf;
    UINT bytes_read;
    fres = f_read(&file, sigBuf.data(), sigBuf.size(), &bytes_read);
    if (fres != FR_OK) {
        return false;
    }
    if (bytes_read != sigBuf.size()) {
        return false;
    }
    // Check signature matches expected
    constexpr std::array<uint8_t, 4> sig_expected = {0xAA, 0x3D, 0x00, 0x01};
    for (std::size_t i = 0; i < sig_expected.size(); i++) {
        if (sig_expected[i] != sigBuf[i]) {
            return false;
        }
    }

    // Read CRC32 from next 4 bytes
    std::array<uint8_t, 4> crcBuf;
    fres = f_read(&file, crcBuf.data(), crcBuf.size(), &bytes_read);
    if (fres != FR_OK) {
        return false;
    }
    if (bytes_read != crcBuf.size()) {
        return false;
    }
    uint32_t crc32_expected = uint32_from_uint8(crcBuf.data());

    // Calculate CRC32 for bin file fw content
    std::array<uint8_t, 1024> flashPageData;
    uint32_t fw_size_bytes = 0;
    uint32_t crc32_calculated = 0;

    // Offset from start of FW file to start of actual FW
    // (Accounting for prefixed CRC etc.)
    // We will need to seek to this point again after calculating CRC
    constexpr uint32_t fw_start_offset = 8;
    fres = f_lseek(&file, fw_start_offset);
    if (fres != FR_OK) {
        return false;
    }

    bool done = false;
    while (!done) {
        UINT bytes_to_read = flashPageData.size();

        flashPageData.fill(0);
        fres = f_read(&file, flashPageData.data(), bytes_to_read, &bytes_read);
        if (fres != FR_OK) {
            return false;
        }

        fw_size_bytes += bytes_read;
        crc32_calculated = crc32iso_hdlc_bit(crc32_calculated, flashPageData.data(), bytes_read);

        if (bytes_read < bytes_to_read) {
            done = true;
        }
    }

    // Check CRC32 matches expected
    if (crc32_calculated != crc32_expected) {
        return false;
    }

    // Check FW will fit in available flash
    // Calculate available flash in bytes
    uint32_t flash_available_bytes = 0;
    for (std::size_t i = 0; i < flashSectors.size(); i++) {
        // First sector is for bootloader
        // This is assumed and should be properly configured somewhere
        if (i == 0) { continue; }
        flash_available_bytes += flashSectors[i].size;
    }
    if (fw_size_bytes > flash_available_bytes) {
        return false;
    }

    // FW has passed all checks. Write FW to flash.

    // Create backup file of currently flashed firmware.
    FIL backup_fw_file;
    static const char backup_fw_filename[] = "firmware_backup.bin";
    fres = f_open(&backup_fw_file, backup_fw_filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (fres != FR_OK) {
        return false;
    }
    UINT bytes_written;
    fres = f_write(&backup_fw_file, (uint8_t*)flashSectors[1].start, flash_available_bytes, &bytes_written);
    if (fres != FR_OK) {
        return false;
    }
    fres = f_close(&backup_fw_file);
    if (fres != FR_OK) {
        return false;
    }

    auto program_flash_from_file = [&]() {

        // Return to start of FW
        FRESULT fres = f_lseek(&file, fw_start_offset);
        if (fres != FR_OK) {
            return false;
        }

        uint32_t currentFlashPage = 0;
        done = false;
        while (!done) {
            // Read FW from file into buffer
            UINT bytes_to_read = flashPageData.size();
            flashPageData.fill(0);
            fres = f_read(&file, flashPageData.data(), bytes_to_read, &bytes_read);
            if (fres != FR_OK) {
                return false;
            }

            // Write buffer to next location in flash
            Write_Flash_Sector(flashPageData, flashSectors, currentFlashPage);
            currentFlashPage += 1;

            if (bytes_read < bytes_to_read) {
                done = true;
            }
        }

        return true;

    };

    if (!program_flash_from_file()) {
        // uh oh, writing to flash has failed for some reason
        // we are in an invalid state and need to recover somehow?
        return false;
    }


    // Read flash contents and calculate CRC32.
    auto calculate_flash_crc = [&]() {
        uint32_t crc_flash_calculated = 0;
        // Assumes flash sectors are contiguous and also byte ordering / alignment. Should handle explicitly.
        crc_flash_calculated = crc32iso_hdlc_bit(crc_flash_calculated, (uint8_t*)flashSectors[1].start, fw_size_bytes);
        return crc_flash_calculated;
    };

    uint32_t crc_flash_calculated = calculate_flash_crc();
    if (crc_flash_calculated != crc32_expected) {
        // uh oh, flash contents don't match expected!
        // we'll try flashing again and recheck
        program_flash_from_file();
        crc_flash_calculated = calculate_flash_crc();
        if (crc_flash_calculated != crc32_expected) {
            // okay, doesn't seem we can fix this
            // need to restore to a safe state
        }
    }


    return true;


}
