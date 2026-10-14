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

void ChipErase(SPI_HandleTypeDef *hspi){
    // We only transmit one byte of data
    // We use 100U (the BMA timout value) as our timeout
    uint8_t opcode = CMD_ERASE; // Chip Erase command
    setChipSelect(0); // Set CS low to start the transaction
    HAL_SPI_Transmit(hspi, &opcode, 1, 100U); 
    setChipSelect(1); // Set CS high to end the transaction
}

void programPage(SPI_HandleTypeDef *hspi, int *pdata, int len, uint32_t addr){

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
    HAL_SPI_Transmit(hspi, packet, len + 4, 100U);

    setChipSelect(1); // Set CS high to end the transaction

}