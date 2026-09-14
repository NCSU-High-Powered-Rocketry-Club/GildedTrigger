/*
 * bma580.c
 *
 * Driver for the Bosch BMA580 accelerometer over I2C.
 */

#include "bma580.h"

/* Timeout applied to each I2C transfer, in milliseconds. */
#define BMA580_I2C_TIMEOUT_MS 100U

/* Set by bma580_init. */
static I2C_HandleTypeDef *bma_i2c;

/*
 * Slave address already shifted left by one for the HAL, which expects the
 * 8-bit form and fills in the read/write bit itself.
 */
static uint16_t bma_addr;

/**
 * @brief Reads one or more sequential registers.
 *
 * The sensor auto-increments the register address across the transfer, so
 * len > 1 reads a contiguous block starting at addr.
 *
 * @param addr   register address to start reading from
 * @param buffer destination for the register contents, at least len bytes
 * @param len    number of bytes to read
 * @retval HAL status, HAL_OK on a successful read
 */
static HAL_StatusTypeDef read_register(uint8_t addr, uint8_t *buffer, uint16_t len);

/**
 * @brief Writes a single register.
 *
 * @param addr register address to write
 * @param data byte to write into that register
 * @retval HAL status, HAL_OK on a successful write
 */
static HAL_StatusTypeDef write_register(uint8_t addr, uint8_t data);

HAL_StatusTypeDef bma580_init(I2C_HandleTypeDef *hi2c, uint8_t dev_addr) {
  if (hi2c == NULL) {
    return HAL_ERROR;
  }
  bma_i2c = hi2c;
  bma_addr = (uint16_t)(dev_addr << 1);
  return HAL_OK;
}

static HAL_StatusTypeDef read_register(uint8_t addr, uint8_t *buffer, uint16_t len) {
  if (bma_i2c == NULL || buffer == NULL) {
    return HAL_ERROR;
  }
  return HAL_I2C_Mem_Read(bma_i2c, bma_addr, addr, I2C_MEMADD_SIZE_8BIT, buffer, len,
                          BMA580_I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef write_register(uint8_t addr, uint8_t data) {
  if (bma_i2c == NULL) {
    return HAL_ERROR;
  }
  return HAL_I2C_Mem_Write(bma_i2c, bma_addr, addr, I2C_MEMADD_SIZE_8BIT, &data, 1,
                           BMA580_I2C_TIMEOUT_MS);
}

void BMA580_Init(){
  // DEVICE COMMS TEST
  // TODO: Wait 3ms (IRM)
  uint8_t latest_byte;
  read_register(0x00, &latest_byte, 0x01); //dummy read 
  read_register(0x00, &latest_byte, 0x01); 
  uint8_t chip_id = latest_byte;

//check chip id is a paticular value

  // Activates altimeter 
  write_register(0x04, 0x00);
  
  // Reset latest byte and check health until it works
  latest_byte = 0x00;
  while(latest_byte != 0x0F){
    read_register(0x04, &latest_byte, 0x00);
  }

  // Turn int 2 pin on
  write_register(0x35, 0x01);

  // Configure power mode to LPM
  uint8_t acc_conf_1;
  read_register(0x31, &acc_conf_1, 0x01);
  acc_conf_1 = acc_conf_1 & 0x7F;
  write_register(0x31, acc_conf_1);
}