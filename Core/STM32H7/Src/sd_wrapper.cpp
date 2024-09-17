#include "sd_wrapper.h"
#include "flash_wrapper.h"
#include "main.h"
#include "fatfs.h"

#include <array>
#include <cstdint>
#include <string>

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

uint32_t crc32_from_sd_file(const char* filename, uint32_t fw_start_offset) {
    FIL file;
    FRESULT fres;
    fres = f_open(&file, filename, FA_READ);
    if (fres != FR_OK) {
        return 0;
    }

    fres = f_lseek(&file, fw_start_offset);
    if (fres != FR_OK) {
        return false;
    }

    std::array<uint8_t, 1024> file_buffer;
    uint32_t crc32_calculated = 0;
    UINT bytes_read;

    bool done = false;
    while (!done) {
        UINT bytes_to_read = file_buffer.size();

        file_buffer.fill(0);
        fres = f_read(&file, file_buffer.data(), bytes_to_read, &bytes_read);
        if (fres != FR_OK) {
            return false;
        }

        crc32_calculated = crc32iso_hdlc_bit(crc32_calculated, file_buffer.data(), bytes_read);

        if (bytes_read < bytes_to_read) {
            done = true;
        }
    }

    fres = f_close(&file);
    if (fres != FR_OK) {
        return 0;
    }

    return crc32_calculated;
}

uint32_t crc_from_flash(std::span<const FlashSector> flashSectors, uint32_t size) {
    uint32_t crc_flash_calculated = 0;
    // Assumes flash sectors are contiguous and also byte ordering / alignment. Should handle explicitly.
    crc_flash_calculated = crc32iso_hdlc_bit(crc_flash_calculated, (uint8_t*)flashSectors.front().start, size);
    return crc_flash_calculated;
}

struct Sd_fw_header {
    uint32_t crc32;
    uint32_t size;
    std::array<uint8_t, 4> signature;
};

Sd_fw_header read_sd_file_header(const char* filename) {

    FIL file;
    FRESULT fres;
    Sd_fw_header header{};
    UINT bytes_read;

    // Open file
    fres = f_open(&file, filename, FA_READ);
    if (fres != FR_OK) {
        return header;
    }

    // Read signature from first 4 bytes
    std::array<uint8_t, 4> sig_buf;
    fres = f_read(&file, sig_buf.data(), sig_buf.size(), &bytes_read);
    if (fres == FR_OK && bytes_read == sig_buf.size()) {
        header.signature = sig_buf;
    } else {
        header.signature.fill(0);
    }

    // Read CRC32 from next 4 bytes
    std::array<uint8_t, 4> crc_buf;
    fres = f_read(&file, crc_buf.data(), crc_buf.size(), &bytes_read);
    if (fres == FR_OK && bytes_read == crc_buf.size()) {
        header.crc32 = uint32_from_uint8(crc_buf.data());
    }

    // Calculate FW actual size
    FILINFO fno;
    fres = f_stat(filename, &fno);
    if (fres == FR_OK) {
        header.size = fno.fsize - 8;
    }

    // Close file
    fres = f_close(&file);
    // if (fres != FR_OK) {
    //     return header;
    // }

    return header;
}

bool check_signatures_match(std::span<const uint8_t> signature, std::span<const uint8_t> reference) {
    //assert(signature.size() == reference.size());
    bool matches = true;

    for (std::size_t i = 0; i < signature.size(); i++) {
        if (signature[i] != reference[i]) {
            matches = false;
        }
    }

    return matches;
}

uint32_t calculate_available_flash_space_bytes(std::span<const FlashSector> flashSectors) {
    uint32_t flash_available_bytes = 0;
    for (const auto& sector : flashSectors) {
        flash_available_bytes += sector.size;
    }
    return flash_available_bytes;
}

bool write_flash_to_file(const char* filename, std::span<const FlashSector> flashSectors) {
    FIL file;
    FRESULT fres;
    bool success = true;
    fres = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (fres != FR_OK) {
        return false;
    }
    UINT bytes_written;
    for (const auto& sector : flashSectors) {
        fres = f_write(&file, (uint8_t*)sector.start, sector.size, &bytes_written);
        if (fres != FR_OK) {
            success = false;
        }
    }
    fres = f_close(&file);
    if (fres != FR_OK) {
        success = false;
    }
    return success;
}

bool write_flash_to_file_retry(const char* filename, std::span<const FlashSector> flashSectors, int max_attempts) {

    bool success = false;
    int attempts = 0;

    uint32_t flash_crc = crc_from_flash(flashSectors, calculate_available_flash_space_bytes(flashSectors));

    while (!success && attempts < max_attempts) {
        // Write flash contents to SD file
        if (!write_flash_to_file(filename, flashSectors)) {
            continue;
        }

        // Verify SD contents against flash CRC
        uint32_t file_crc = crc32_from_sd_file(filename, 0);
        if (flash_crc == file_crc) {
            success = true;
        }
    }

    return success;

}

uint32_t write_file_to_flash(const char* filename, uint32_t fw_start_offset, std::span<const FlashSector> flashSectors) {

    FIL file;
    FRESULT fres;

    // Open file
    fres = f_open(&file, filename, FA_READ);
    if (fres != FR_OK) {
        return 0;
    }

    // Jump to start of FW
    fres = f_lseek(&file, fw_start_offset);
    if (fres != FR_OK) {
        return 0;
    }

    uint32_t currentFlashPage = 0;
    uint32_t bytes_written = 0;
    std::array<uint8_t, 1024> file_buffer;
    UINT bytes_read;
    bool done = false;
    while (!done) {
        // Read FW from file into buffer
        UINT bytes_to_read = file_buffer.size();
        file_buffer.fill(0);
        fres = f_read(&file, file_buffer.data(), bytes_to_read, &bytes_read);
        if (fres != FR_OK) {
            return 0;
        }

        // Write buffer to next location in flash
        Write_Flash_Sector(file_buffer, flashSectors, currentFlashPage);
        currentFlashPage += 1;
        bytes_written += bytes_read;

        if (bytes_read < bytes_to_read) {
            done = true;
        }
    }

    return bytes_written;
}

bool write_file_to_flash_retry(const char* filename, uint32_t fw_start_offset, std::span<const FlashSector> flashSectors, int max_attempts, uint32_t good_crc) {

    bool success = false;
    int attempts = 0;

     while (!success && attempts < max_attempts) {

        // Write firmware file to flash
        uint32_t bytes_written = write_file_to_flash(filename, fw_start_offset, flashSectors);
        if (bytes_written == 0) {
            // uh oh, writing to flash has failed for some reason
            // we are in an invalid state and need to recover somehow
            continue;
        }

        // Check flash CRC matches expected
        uint32_t flash_crc = crc_from_flash(flashSectors.subspan(1), bytes_written);
        if (flash_crc == good_crc) {
            success = true;
        }
     }

    return success;
}

bool create_marker_file(const char* filename) {
    FIL file;
    FRESULT fres;
    bool success = true;
    fres = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (fres != FR_OK) {
        return false;
    }
    fres = f_close(&file);
    if (fres != FR_OK) {
        success = false;
    }
    return success;
}

void log_message(std::string msg) {
    FIL file_log;
    std::string log_filename = "sd_fw_log.txt";
    f_open(&file_log, log_filename.c_str(), FA_OPEN_APPEND | FA_WRITE);
    f_printf(&file_log, "%8ul - %s\n", HAL_GetTick(), msg.c_str());
    f_close(&file_log);
}


bool attempt_install_from_Sd(std::span<const FlashSector> flashSectors) {

    // 1 - Initialise SD card access (if available)
    //       If yes, continue to step (2). If not, abort.
    // 2 - Check if a DONE marker file is present.
    //       If not, continue to step (3). If so, abort.
    // 3 - Check if a valid FW update file is available.
    //       Valid means:
    //         Filename as expected
    //         Signed as expected
    //         Header CRC matches payload
    //         Size will fit in available flash space
    //       If yes, continue to step (4). If not, abort.
    // 4 - Dump the currently flashed FW to a backup file on the SD card.
    //       If fails, abort. If succeeds, continue to step (5).
    // 5 - Write the new FW to flash.
    //       If writing fails, proceed to recovery step (9).
    //       If writing succeeds, proceed to step (6).
    // 6 - Read flash and check if matches new FW's CRC.
    //       If CRC check fails, proceed to recovery step (9).
    //       If CRC passes, proceed to step (7).
    // 7 - Write DONE marker file to indicate FW update is complete.
    //       Proceed to step (8).
    // 8 - Boot Firmware.
    // 9 - RECOVERY WRITE. Write the backup FW to flash.
    //       If write fails, proceed to failsafe step (11).
    //       If write succeeds, proceed to step (10).
    // 10 - RECOVERY CHECK. Read flash and check if matches backup FW's CRC.
    //       If CRC check fails, proceed to failsafe step (11).
    //       If CRC passes, proceed to step (8).
    // 11 - FAILSAFE. Erase application flash to ensure corrupted FW does not run.
    //       Proceed to run bootloader waiting for USB connection.


    const char new_fw_filename[] = "sd_fw.bin"; // sd_fw_nosig
    const char backup_fw_filename[] = "sd_fw.bak.bin";
    const char update_done_marker_filename[] = "sd_fw.done";
    constexpr uint32_t new_fw_offset = 8;
    constexpr std::array<uint8_t, 4> sig_expected = {0xAA, 0x3D, 0x00, 0x01};

    MX_SPI4_Init();
    MX_FATFS_Init();

    FATFS FatFs;
    FRESULT fres;

    // Open the file system
    fres = f_mount(&FatFs, "", 1 /* 1 = mount now */);
    if (fres != FR_OK) {
        return false;
    }

    // Check if a DONE marker file exists and exit if so
    if (f_stat(update_done_marker_filename, nullptr) != FR_NO_FILE) {
        return false;
    }

    // Check if a FW update file exists
    if (f_stat(new_fw_filename, nullptr) == FR_NO_FILE) {
        return false;
    }

    log_message("Firmware update file found. Checking...");

    // Read header metadata from file
    auto header = read_sd_file_header(new_fw_filename);

    // Check signature matches expected
    if (!check_signatures_match(header.signature, sig_expected)) {
        log_message("Firmware update file signature: FAIL");
        return false;
    }
    log_message("Firmware update file signature: PASS");

    // Calculate CRC32 of SD file firmware contents
    uint32_t crc32_calculated = crc32_from_sd_file(new_fw_filename, new_fw_offset);
    // Check CRC32 matches header value
    if (crc32_calculated != header.crc32) {
        log_message("Firmware update file CRC32: FAIL");
        return false;
    }
    log_message("Firmware update file CRC32: PASS");

    // Check FW will fit in available flash
    uint32_t flash_available_bytes = calculate_available_flash_space_bytes(flashSectors.subspan(1));
    if (header.size > flash_available_bytes) {
        log_message("Firmware update size check: FAIL");
        return false;
    }
    log_message("Firmware update size check: PASS");

    // FW has passed all checks. Write FW to flash.

    // FIL file_log;
    // //FRESULT fres;
    // std::string log_filename = "sd_fw_log.txt";
    // fres = f_open(&file_log, log_filename.c_str(), FA_CREATE_ALWAYS | FA_WRITE);
    // f_printf(&file_log, "%ul - starting SD firmware update...\n", HAL_GetTick());


    // Create backup file of currently flashed firmware.
    log_message("Firmware update passes checks! Firmware will be updated.");
    log_message("Creating backup of current firmware...");
    if (!write_flash_to_file_retry(backup_fw_filename, flashSectors.subspan(1), 5)) {
        log_message("Firmware backup: FAIL");
        return false;
    }
    log_message("Firmware backup: PASS");

    // After this point, we are modifying flash contents.

    // Attempt to install the new FW from file with X retries
    log_message("Writing updated firmware...");
    bool new_fw_success = write_file_to_flash_retry(new_fw_filename, new_fw_offset, flashSectors, 5, header.crc32);

    // If successful, create a marker file so that FW update doesn't re-run
    if (new_fw_success) {
        log_message("Firmware update: PASS");
        create_marker_file(update_done_marker_filename);
    } else {
        log_message("Firmware update: FAIL");
        log_message("Firmware update has not succeeded!");
    }

    // Something went wrong
    if (!new_fw_success) {

        // Attempt to recover by flashing backup of original firmware with X retries
        log_message("Restoring backup firmware...");
        uint32_t backup_crc = crc32_from_sd_file(backup_fw_filename, 0);
        bool backup_fw_success = write_file_to_flash_retry(backup_fw_filename, 0, flashSectors, 5, backup_crc);

        if (backup_fw_success) {
            log_message("Firmware backup restore: PASS");
        } else {
            log_message("Firmware backup restore: FAIL");
            log_message("Firmware backup restore has not succeeded! Firmware is in an invalid state. Please restore firmware via USB.");

            // Backup FW install has failed.
            // We will wipe start of flash to prevent running of potentially corrupt firmware.
            std::array<uint8_t, 1024> file_buffer;
            file_buffer.fill(0);
            Write_Flash_Sector(file_buffer, flashSectors, 0);
        }
    }

    return new_fw_success;
}
