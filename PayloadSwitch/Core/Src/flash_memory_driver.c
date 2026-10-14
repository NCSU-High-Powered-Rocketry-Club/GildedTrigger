void setChipSelect(bool high){
    if(high){
        // Set high
        HAL_GPIO_WritePin(Flash_CS_GPIO_Port, Flash_CS_Pin, GPIO_PIN_SET);
    } else {
        // Set low
        HAL_GPIO_WritePin(Flash_CS_GPIO_Port, Flash_CS_Pin, GPIO_PIN_RESET);
    }
}

void ChipErase(SPI_HandleTypeDef *hspi, int *pdata){
    // We only transmit one byte of data
    // We use 100U (the BMA timout value) as our timeout
    HAL_SPI_Transmit(hspi, pdata, 1, 100U);
}