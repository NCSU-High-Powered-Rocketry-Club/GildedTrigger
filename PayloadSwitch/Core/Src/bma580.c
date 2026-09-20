/*
 * bma580.c
 *
 * Driver for the Bosch BMA580 accelerometer over I2C.
 */

#include "bma580.h"

/* Timeout applied to each I2C transfer, in milliseconds. */
#define BMA580_I2C_TIMEOUT_MS 100U

/* The sensor needs this long after power-up before it will answer on the bus. */
#define BMA580_POWER_ON_DELAY_MS 3U

/* Time allowed for the sensor to report ready during configuration. */
#define BMA580_STATUS_TIMEOUT_MS 100U

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

/**
 * @brief Re-reads a register until the masked bits match, or time runs out.
 *
 * @param addr       register address to poll
 * @param mask       bits of the register to compare
 * @param expected   value those bits must take
 * @param timeout_ms how long to keep polling before giving up
 * @retval HAL_OK once the bits match, HAL_TIMEOUT if they never do, or the
 *         failing HAL status if a read errors out
 */
static HAL_StatusTypeDef poll_register(uint8_t addr, uint8_t mask, uint8_t expected,
                                       uint32_t timeout_ms);

HAL_StatusTypeDef setup_bma580(I2C_HandleTypeDef *hi2c, uint8_t dev_addr) {
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

static HAL_StatusTypeDef poll_register(uint8_t addr, uint8_t mask, uint8_t expected,
                                       uint32_t timeout_ms) {
  uint32_t start = HAL_GetTick();

  for (;;) {
    uint8_t value;
    HAL_StatusTypeDef status = read_register(addr, &value, 1);
    if (status != HAL_OK) {
      return status;
    }
    if ((value & mask) == expected) {
      return HAL_OK;
    }
    if ((HAL_GetTick() - start) > timeout_ms) {
      return HAL_TIMEOUT;
    }
  }
}

float getAcceleration(void){
    uint8_t values[6];
    float xAcceleration, yAcceleration, zAcceleration;
    read_register(0x00, values, 0x06);

    // Get 2s complement of x, y and z axis data
    int16_t x2s = (int16_t)((values[1] << 8) | values[0]);
    int16_t y2s = (int16_t)((values[3] << 8) | values[2]);
    int16_t z2s = (int16_t)((values[5] << 8) | values[4]);

    xAcceleration = 0.0f;
    // Convert x to signed integer
    for(int i = 1; i < 16; i++){
      // process x bits one at a time
      int bit = x2s >> (16-i);
      // Account for negative weighting of Most Significant Bit
      xAcceleration += (i != 15) ? pow(bit, i-1) : -pow(bit, i-1);
    }
    // Convert number to right metric by multiplying by range and dividing by 2^16
    xAcceleration = xAcceleration * 16 / 32768;

    yAcceleration = 0.0f;
    // Convert y to signed integer
    for(int i = 1; i < 16; i++){
      // process y bits one at a time
      int bit = y2s >> (16-i);
      // Account for negative weighting of Most Significant Bit
      yAcceleration += (i != 15) ? pow(bit, i-1) : -pow(bit, i-1);
    }
    // Convert number to right metric by multiplying by range and dividing by 2^16
    yAcceleration = yAcceleration * 16 / 32768;

    zAcceleration = 0.0f;
    // Convert z to signed integer
    for(int i = 1; i < 16; i++){
      // process z bits one at a time
      int bit = z2s >> (16-i);
      // Account for negative weighting of Most Significant Bit
      zAcceleration += (i != 15) ? pow(bit, i-1) : -pow(bit, i-1);
    }
    // Convert number to right metric by multiplying by range and dividing by 2^16
    zAcceleration = zAcceleration * 16 / 32768;

    // The magnitude of the accelertion vector
    // TODO: convert to g units
    // Seth: I think it's already in Gs. I converted each acceleration vector to Gs after converting to 2's complement.
    return sqrt(pow(xAcceleration, 2) + pow(yAcceleration, 2) + pow(zAcceleration, 2));
}

HAL_StatusTypeDef BMA580_Init(void) {
  HAL_StatusTypeDef status;
  uint8_t chip_id;

  HAL_Delay(BMA580_POWER_ON_DELAY_MS);

  /* The first read after power-up returns undefined data, so discard it. */
  status = read_register(0x00, &chip_id, 1);
  if (status != HAL_OK) {
    return status;
  }
  status = read_register(0x00, &chip_id, 1);
  if (status != HAL_OK) {
    return status;
  }
  /* TODO: reject chip_id values other than the one the datasheet specifies. */

  // Activates altimeter
  status = write_register(0x04, 0x00);
  if (status != HAL_OK) {
    return status;
  }

  // Check health until it works
  status = poll_register(0x04, 0xFF, 0x0F, BMA580_STATUS_TIMEOUT_MS);
  if (status != HAL_OK) {
    return status;
  }

  // Turn int 2 pin on for I2C
  status = write_register(0x35, 0x01);
  if (status != HAL_OK) {
    return status;
  }

  // Configure Sensor Parameters
  status = write_register(0x30, 0b00000000); // disable accelerometer to change config
  if (status != HAL_OK) {
    return status;
  }
  status = write_register(0x31, 0b00101000); // ODR 400Hz, Average 4 samples, Low Power Mode
  if (status != HAL_OK) {
    return status;
  }
  status = write_register(0x32, 0b00001111); // +/- 16g, -60dB roll-off, low noise mode, status flag of acc_drdy_int is not cleared automatically.
  if (status != HAL_OK) {
    return status;
  }
  status = write_register(0x30, 0b00001111); // Enable accelerometer and temp sensor
  if (status != HAL_OK) {
    return status;
  }

  // Wait for sensor to be ready
  return poll_register(0x11, 0x01, 0x01, BMA580_STATUS_TIMEOUT_MS);
}