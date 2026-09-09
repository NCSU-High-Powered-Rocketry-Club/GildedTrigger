#include "stm32u0xx_hal_i2c.c";

// This will be set in the init function in main
static hal_i2c_handle_t * handle;

static HAL_StatusTypeDef read_registers(hal_i2c_handle_t * hi2cHandle, uint8_t addr, uint32_t mem_addr , hal_i2c_mem_addr_size_t mem_size , void * p_data , uint32_t size_byte , uint32_t timeout_ms ){
    // I think this shifts the adress to left
    addr |= 0x80;
    return HAL_I2C_Mem_Read(hi2cHandle , addr, mem_addr , mem_size, &p_data_buffer , size_byte, timeout_ms);
}
