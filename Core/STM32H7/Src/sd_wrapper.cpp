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

const uint32_t crc32lookup[256] = {
    0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,
    0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
    0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,
    0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
    0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,
    0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
    0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
    0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
    0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,
    0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
    0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,
    0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
    0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,
    0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
    0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
    0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
    0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,
    0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
    0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,
    0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
    0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
    0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
    0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,
    0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
    0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,
    0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
    0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,
    0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
    0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,
    0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
    0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,
    0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D,
  };

uint32_t crc32_1byte(uint32_t prev_crc, uint8_t const *mem, size_t len)
{
  uint32_t crc = ~prev_crc;
  unsigned char* current = (unsigned char*) mem;
  uint32_t length = len;
  while (length--)
    crc = (crc >> 8) ^ crc32lookup[(crc & 0xFF) ^ *current++];
  return ~crc;
}

uint32_t crc32_hw(uint32_t crc, uint8_t const *mem, size_t len) {

    // CRC_HandleTypeDef   CrcHandle;
    // CrcHandle.Instance = CRC;

    // /* The default polynomial is used */
    // CrcHandle.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_ENABLE;
    // // CrcHandle.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_DISABLE;
    // // CrcHandle.Init.GeneratingPolynomial = 0xedb88320;
    // // CrcHandle.Init.CRCLength = CRC_POLYLENGTH_32B;

    // /* The default init value is used */
    // CrcHandle.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_DISABLE;
    // //CrcHandle.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_ENABLE;
    // CrcHandle.Init.InitValue = ~crc;

    // /* The input data are not inverted */
    // CrcHandle.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_BYTE;

    // /* The output data are not inverted */
    // CrcHandle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;

    // /* The input data are 32-bit long words */
    // CrcHandle.InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES;

    // if (HAL_CRC_Init(&CrcHandle) != HAL_OK)
    // {
    //     /* Initialization Error */
    //     Error_Handler();
    // }

    __HAL_RCC_CRC_CLK_ENABLE();
    CRC->CR = 0;
    //CRC->CR |= CRC_CR_REV_IN_0;
    CRC->CR |= CRC_CR_REV_IN_1;
    CRC->CR |= CRC_CR_REV_OUT;
    CRC->CR |= CRC_CR_RESET;

    CRC->DR = crc;

    uint32_t i = 0;
    while (i < len) {
        CRC->DR = *(uint32_t*)&mem[i];
        i += 4;
    }

    uint32_t val = CRC->DR ^ 0xFFFFFFFF;
    __HAL_RCC_CRC_CLK_DISABLE();

    return val;


    //uint32_t uwCRCValue = 0;//HAL_CRC_Calculate(&CrcHandle, (uint32_t *)mem, len);
    // uwCRCValue = ~uwCRCValue;
    //return uwCRCValue;

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
    uint32_t crc_start = HAL_GetTick();

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
    uint32_t crc32_calculated_hw = 0;
    UINT bytes_read;

    bool done = false;
    while (!done) {
        UINT bytes_to_read = file_buffer.size();

        file_buffer.fill(0);
        fres = f_read(&file, file_buffer.data(), bytes_to_read, &bytes_read);
        if (fres != FR_OK) {
            return false;
        }

        //crc32_calculated = crc32iso_hdlc_bit(crc32_calculated, file_buffer.data(), bytes_read);
        //crc32_calculated_hw = crc32_hw(crc32_calculated_hw, file_buffer.data(), bytes_read);
        crc32_calculated = crc32_1byte(crc32_calculated, file_buffer.data(), bytes_read);

        if (bytes_read < bytes_to_read) {
            done = true;
        }
    }

    fres = f_close(&file);
    if (fres != FR_OK) {
        return 0;
    }


    uint32_t crc_stop = HAL_GetTick();
    uint32_t crc_duration = crc_stop - crc_start;

    return crc32_calculated;
}

uint32_t crc_from_flash(std::span<const FlashSector> flashSectors, uint32_t size) {
    uint32_t crc_flash_calculated = 0;
    // Assumes flash sectors are contiguous and also byte ordering / alignment. Should handle explicitly.
    // uint32_t crc_start = HAL_GetTick();
    // crc_flash_calculated = crc32iso_hdlc_bit(crc_flash_calculated, (uint8_t*)flashSectors.front().start, size);
    // uint32_t crc_sw_done = HAL_GetTick();
    crc_flash_calculated = crc32_1byte(crc_flash_calculated, (uint8_t*)flashSectors.front().start, size);
    // uint32_t crc_sw2_done = HAL_GetTick();
    // uint32_t sw_duration = crc_sw_done - crc_start;
    // uint32_t sw2_duration = crc_sw2_done - crc_sw_done;
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
    uint32_t write_start = HAL_GetTick();
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
    uint32_t write_end = HAL_GetTick();
    uint32_t write_duration = write_end - write_start;
    return success;
}

bool write_flash_to_file_retry(const char* filename, std::span<const FlashSector> flashSectors, int max_attempts) {

    bool success = false;
    int attempts = 0;

    uint32_t crc_start = HAL_GetTick();
    uint32_t flash_crc = crc_from_flash(flashSectors, calculate_available_flash_space_bytes(flashSectors));
    uint32_t crc_stop = HAL_GetTick();
    uint32_t crc_duration = crc_stop - crc_start;

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
    f_printf(&file_log, "%8u - %s\n", HAL_GetTick(), msg.c_str());
    f_close(&file_log);
}



void crc_test() {
    #define BUFFER_SIZE    114
    static const uint32_t aDataBuffer[BUFFER_SIZE] =
    {
    0x00001021, 0x20423063, 0x408450a5, 0x60c670e7, 0x9129a14a, 0xb16bc18c,
    0xd1ade1ce, 0xf1ef1231, 0x32732252, 0x52b54294, 0x72f762d6, 0x93398318,
    0xa35ad3bd, 0xc39cf3ff, 0xe3de2462, 0x34430420, 0x64e674c7, 0x44a45485,
    0xa56ab54b, 0x85289509, 0xf5cfc5ac, 0xd58d3653, 0x26721611, 0x063076d7,
    0x569546b4, 0xb75ba77a, 0x97198738, 0xf7dfe7fe, 0xc7bc48c4, 0x58e56886,
    0x78a70840, 0x18612802, 0xc9ccd9ed, 0xe98ef9af, 0x89489969, 0xa90ab92b,
    0x4ad47ab7, 0x6a961a71, 0x0a503a33, 0x2a12dbfd, 0xfbbfeb9e, 0x9b798b58,
    0xbb3bab1a, 0x6ca67c87, 0x5cc52c22, 0x3c030c60, 0x1c41edae, 0xfd8fcdec,
    0xad2abd0b, 0x8d689d49, 0x7e976eb6, 0x5ed54ef4, 0x2e321e51, 0x0e70ff9f,
    0xefbedfdd, 0xcffcbf1b, 0x9f598f78, 0x918881a9, 0xb1caa1eb, 0xd10cc12d,
    0xe16f1080, 0x00a130c2, 0x20e35004, 0x40257046, 0x83b99398, 0xa3fbb3da,
    0xc33dd31c, 0xe37ff35e, 0x129022f3, 0x32d24235, 0x52146277, 0x7256b5ea,
    0x95a88589, 0xf56ee54f, 0xd52cc50d, 0x34e224c3, 0x04817466, 0x64475424,
    0x4405a7db, 0xb7fa8799, 0xe75ff77e, 0xc71dd73c, 0x26d336f2, 0x069116b0,
    0x76764615, 0x5634d94c, 0xc96df90e, 0xe92f99c8, 0xb98aa9ab, 0x58444865,
    0x78066827, 0x18c008e1, 0x28a3cb7d, 0xdb5ceb3f, 0xfb1e8bf9, 0x9bd8abbb,
    0x4a755a54, 0x6a377a16, 0x0af11ad0, 0x2ab33a92, 0xed0fdd6c, 0xcd4dbdaa,
    0xad8b9de8, 0x8dc97c26, 0x5c644c45, 0x3ca22c83, 0x1ce00cc1, 0xef1fff3e,
    0xdf7caf9b, 0xbfba8fd9, 0x9ff86e17, 0x7e364e55, 0x2e933eb2, 0x0ed11ef0
    };

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    uint32_t start = 0;

    start = DWT->CYCCNT;
    uint32_t crc_hw_val = crc32_hw(0xFFFFFFFF, (uint8_t *)aDataBuffer, BUFFER_SIZE);
    uint32_t crc32_hw_duration = DWT->CYCCNT - start;

    start = DWT->CYCCNT;
    uint32_t crc_sw_val = crc32iso_hdlc_bit(0xFFFFFFFF, (uint8_t *)aDataBuffer, BUFFER_SIZE);
    uint32_t crc32iso_hdlc_bit_duration = DWT->CYCCNT - start;

    // const uint32_t Polynomial = 0xEDB88320;
    // for (unsigned int i = 0; i <= 0xFF; i++) {
    //     uint32_t crc = i;
    //     for (unsigned int j = 0; j < 8; j++) { crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial); }
    //     crc32lookup[i] = crc;
    // }

    start = DWT->CYCCNT;
    uint32_t crc_sw_lut1_val = crc32_1byte(0xFFFFFFFF, (uint8_t *)aDataBuffer, BUFFER_SIZE);
    uint32_t crc_sw_lut1_val_duration = DWT->CYCCNT - start;
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

    __HAL_RCC_CRC_CLK_ENABLE();

    crc_test();

    FATFS FatFs;
    FRESULT fres;

    // Open the file system
    HAL_Delay(1000);
    fres = f_mount(&FatFs, "", 1 /* 1 = mount now */);
    if (fres != FR_OK) {
        return false;
    }

    // Check if a DONE marker file exists and exit if so
    // if (f_stat(update_done_marker_filename, nullptr) != FR_NO_FILE) {
    //     return false;
    // }

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

        /* Enable CRC reset state */
    __HAL_RCC_CRC_FORCE_RESET();

    /* Release CRC from reset state */
    __HAL_RCC_CRC_RELEASE_RESET();

    return new_fw_success;
}
