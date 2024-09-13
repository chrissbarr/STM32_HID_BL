#include "sd_wrapper.h"
#include "main.h"
#include "fatfs.h"

#include <array>

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


bool Initialise_SD() {

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
    static const char filename[] = "h7_fw_update.bin";
    FIL file;
    fres = f_open(&file, filename, FA_READ);
    if (fres != FR_OK) {
        return false;
    }

    std::array<uint8_t, 1024> flashPageData;
    int size_bytes = 0;

    bool done = false;
    while (!done) {
        UINT bytes_to_read = flashPageData.size();
        UINT bytes_read;
        flashPageData.fill(0);
        fres = f_read(&file, flashPageData.data(), bytes_to_read, &bytes_read);
        if (fres != FR_OK) {
            return false;
        }

        size_bytes += bytes_read;

        if (bytes_read < bytes_to_read) {
            done = true;
        }
    }

    // //Let's get some statistics from the SD card
    // DWORD free_clusters, free_sectors, total_sectors;

    // FATFS* getFreeFs;

    // fres = f_getfree("", &free_clusters, &getFreeFs);
    // if (fres != FR_OK) {
    //     //myprintf("f_getfree error (%i)\r\n", fres);
    //     while(1);
    // }

    // //Formula comes from ChaN's documentation
    // total_sectors = (getFreeFs->n_fatent - 2) * getFreeFs->csize;
    // free_sectors = free_clusters * getFreeFs->csize;

    // //myprintf("SD card stats:\r\n%10lu KiB total drive space.\r\n%10lu KiB available.\r\n", total_sectors / 2, free_sectors / 2);

    // //Now let's try to open file "test.txt"
    // fres = f_open(&fil, "test.txt", FA_READ);
    // if (fres != FR_OK) {
    //     //myprintf("f_open error (%i)\r\n");
    //     while(1);
    // }
    // //myprintf("I was able to open 'test.txt' for reading!\r\n");

    //

    return true;


}
