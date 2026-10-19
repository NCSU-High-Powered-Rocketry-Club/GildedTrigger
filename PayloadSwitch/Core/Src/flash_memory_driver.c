#include "flash_memory_driver.h"

/** 
 * @brief Waits for the flash memory to be ready for the next operation.
 * @param hspi Pointer to the SPI handle
 * @param timeout_ms Timeout in milliseconds
 * @retval HAL_OK if the flash memory is ready, HAL_TIMEOUT if it takes too long
*/
static HAL_StatusTypeDef waitReady(SPI_HandleTypeDef *hspi, uint32_t timeout_ms);

/** 
 * @brief reads the status register of the flash memory.
 * @param hspi Pointer to the SPI handle
 * @retval The value of the status register
*/
static uint8_t readStatus(SPI_HandleTypeDef *hspi);

/** 
 * @brief Enables writing to the flash memory.
 * @param hspi Pointer to the SPI handle
 * @retval HAL_OK if successful, HAL_ERROR if failed
*/
static HAL_StatusTypeDef writeEnable(SPI_HandleTypeDef *hspi);

void setChipSelect(bool high){
    if(high){
        // Set high
        HAL_GPIO_WritePin(Flash_CS_GPIO_Port, Flash_CS_Pin, GPIO_PIN_SET);
    } else {
        // Set low
        HAL_GPIO_WritePin(Flash_CS_GPIO_Port, Flash_CS_Pin, GPIO_PIN_RESET);
    }
}

HAL_StatusTypeDef ChipErase(SPI_HandleTypeDef *hspi){
    // We only transmit one byte of data
    uint8_t cmd = CMD_ERASE; // Chip Erase command
    HAL_StatusTypeDef status = writeEnable(hspi); // Enable writing to the flash memory
    if (status != HAL_OK) return status;

    setChipSelect(0); // Set CS low to start the transaction
    status = HAL_SPI_Transmit(hspi, &cmd, 1, MX25R3235F_ERASE_TIMEOUT); 
    setChipSelect(1); // Set CS high to end the transaction
    if (status != HAL_OK) return status;

    status = waitReady(hspi, MX25R3235F_ERASE_TIMEOUT); // Wait for the chip to finish erasing
    if (status != HAL_OK) return status;

    return HAL_OK;
    
}

static uint8_t readStatus(SPI_HandleTypeDef *hspi){
    //read status register
    uint8_t statusReg = 0xFF; //initialize to 0xFF so that if the read corrupts it wont continue
    uint8_t cmd = CMD_RDSR;

    setChipSelect(0);
    HAL_SPI_Transmit(hspi, &cmd, 1, MX25R3235F_PAGE_TIMEOUT);
    HAL_StatusTypeDef status = HAL_SPI_Receive(hspi, &statusReg, 1, MX25R3235F_PAGE_TIMEOUT); //receive the 1 byte status register into statusReg
    setChipSelect(1);
    if (status != HAL_OK) return status;

    return statusReg;
}

static HAL_StatusTypeDef waitReady(SPI_HandleTypeDef *hspi, uint32_t timeout_ms){
    //wait for the flash memory to be ready
    uint32_t start = HAL_GetTick();
    while(readStatus(hspi) & 0x01){ //check the WIP bit of the status register p30
        if((HAL_GetTick() - start) > timeout_ms){
            return HAL_TIMEOUT; //timeout if it takes too long
        }
    }
    return HAL_OK;
}


static HAL_StatusTypeDef writeEnable(SPI_HandleTypeDef *hspi){
    //write enable command
    HAL_StatusTypeDef status;
    uint8_t cmd = CMD_WRITE_ENABLE;
    setChipSelect(0);

    status = HAL_SPI_Transmit(hspi, &cmd, 1, MX25R3235F_PAGE_TIMEOUT);

    setChipSelect(1);
    return status;
}

HAL_StatusTypeDef deepSleep(SPI_HandleTypeDef *hspi){
    //send deep sleep command
    uint8_t cmd = CMD_DEEPSLEEP;
    setChipSelect(0);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(hspi, &cmd, 1, MX25R3235F_PAGE_TIMEOUT);
    setChipSelect(1);
    return status;
}

void wakeUp(SPI_HandleTypeDef *hspi){
    //send wake up command
    setChipSelect(0);
    setChipSelect(1);
    HAL_Delay(1); //wait 1 ms for the flash memory to wake up
}

HAL_StatusTypeDef programPage(SPI_HandleTypeDef *hspi, const uint8_t *pdata, int len, uint32_t addr){

    if (pdata == NULL || len <= 0 || (addr & 0xFF) + len > 256) { //(addr & 0xFF) + len > 256 checks if the data will be written across a page boundary, which is not allowed
        return HAL_ERROR; // Invalid parameters
    }

    HAL_StatusTypeDef status = writeEnable(hspi); // Enable writing to the flash memory
    if (status != HAL_OK) return status;

    
    uint8_t packet[len + 4]; // Create a packet with the command and data
    packet[0] = CMD_PAGEPROGRAM; // Set the first byte to the Page Program command
    // Copy the address into the packet
    packet[1] = (addr >> 16) & 0xFF;
    packet[2] = (addr >> 8) & 0xFF;
    packet[3] = addr & 0xFF;
    for(int i = 0; i < len; i++){
        packet[i + 4] = pdata[i]; // Copy the data into the packet
    }

    setChipSelect(0); // Set CS low to start the transaction
    status = HAL_SPI_Transmit(hspi, packet, len + 4, MX25R3235F_PAGE_TIMEOUT);
    setChipSelect(1); // Set CS high to end the transaction
    if (status != HAL_OK) return status;

    status = waitReady(hspi, MX25R3235F_PAGE_TIMEOUT); // Wait for the chip to finish programming
    if (status != HAL_OK) return status;

    return HAL_OK;
}