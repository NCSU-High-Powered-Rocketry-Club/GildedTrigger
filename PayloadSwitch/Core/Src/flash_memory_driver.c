#include "flash_memory_driver.h"

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
    HAL_StatusTypeDef status;
    uint8_t cmd = CMD_ERASE; // Chip Erase command
    setChipSelect(0); // Set CS low to start the transaction

    status = HAL_SPI_Transmit(hspi, &cmd, 1, MX25R3235F_ERASE_TIMEOUT); 

    setChipSelect(1); // Set CS high to end the transaction
    return status;
}

static uint8_t readStatus(SPI_HandleTypeDef *hspi){
    //read status register
    uint8_t statusReg;
    setChipSelect(0);

    uint8_t cmd = CMD_RDSR;
    HAL_SPI_Transmit(hspi, &cmd, 1, MX25R3235F_PAGE_TIMEOUT);
    HAL_SPI_Receive(hspi, &statusReg, 1, MX25R3235F_PAGE_TIMEOUT); //recieve the 1 byte status register into statusReg
    
    setChipSelect(1);
    return statusReg;
}

static HAL_StatusTypeDef waitReady(SPI_HandleTypeDef *hspi){
    //wait for the flash memory to be ready
    uint32_t start = HAL_GetTick();
    while(readStatus(hspi) & 0x01){ //check the WIP bit of the status register p30
        HAL_Delay(1); //wait 1 ms before checking again
        if((HAL_GetTick() - start) > MX25R3235F_ERASE_TIMEOUT){
            return HAL_TIMEOUT; //timeout if it takes too long
        }
    }
    return HAL_OK;
}


static HAL_StatusTypeDef writeEnable(SPI_HandleTypeDef *hspi){
    //write enable command
    HAL_StatusTypeDef status;
    uint8_t cmd = CMD_WRITE_ENABLE;
    waitReady(hspi); //wait for the flash memory to be ready
    setChipSelect(0);

    status = HAL_SPI_Transmit(hspi, &cmd, 1, MX25R3235F_PAGE_TIMEOUT);

    setChipSelect(1);
    return status;
}

void deepSleep(SPI_HandleTypeDef *hspi){
    //send deep sleep command
    uint8_t cmd = CMD_DEEPSLEEP;
    setChipSelect(0);
    HAL_SPI_Transmit(hspi, &cmd, 1, MX25R3235F_PAGE_TIMEOUT);
    setChipSelect(1);
}

void wakeUp(SPI_HandleTypeDef *hspi){
    //send wake up command
    uint8_t cmd = CMD_DEEPSLEEP;
    setChipSelect(0);
    HAL_Delay(1); //wait 1 ms for the flash memory to wake up
    setChipSelect(1);
}

HAL_StatusTypeDef programPage(SPI_HandleTypeDef *hspi, const uint8_t *pdata, int len, uint32_t addr){

    writeEnable(hspi); // Enable writing to the flash memory

    setChipSelect(0); // Set CS low to start the transaction

    uint8_t packet[len + 4]; // Create a packet with the command and data
    packet[0] = CMD_PAGEPROGRAM; // Set the first byte to the Page Program command
    // Copy the address into the packet
    packet[1] = (addr >> 16) & 0xFF;
    packet[2] = (addr >> 8) & 0xFF;
    packet[3] = addr & 0xFF;
    for(int i = 0; i < len; i++){
        packet[i + 4] = pdata[i]; // Copy the data into the packet
    }
    HAL_StatusTypeDef status = HAL_SPI_Transmit(hspi, packet, len + 4, MX25R3235F_PAGE_TIMEOUT);

    setChipSelect(1); // Set CS high to end the transaction

    return status;
}