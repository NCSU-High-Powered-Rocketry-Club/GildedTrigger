void setChipSelect(bool high){
    if(high){
        // Set high
        HAL_GPIO_WritePin(Flash_CS_GPIO_Port, Flash_CS_Pin, GPIO_PIN_RESET);
    } else {
        // Set low
    }
}