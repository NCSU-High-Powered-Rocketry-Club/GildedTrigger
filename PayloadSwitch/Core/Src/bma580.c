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

/* Expected contents of the CHIP_ID register (0x00). */
#define BMA580_CHIP_ID 0xC4U

/* Time allowed for the sensor to report ready during configuration. */
#define BMA580_STATUS_TIMEOUT_MS 100U

/* Set by setup_bma580. */
static I2C_HandleTypeDef *bma_i2c;

/* Set up I2C handle*/
static I2C_HandleTypeDef i2c_handle;

/* Raised from interrupt context when the sensor asserts its INT pin. */
static volatile uint8_t bma_interrupt;

/*
 * Slave address already shifted left by one for the HAL, which expects the
 * 8-bit form and fills in the read/write bit itself.
 */
static uint8_t bma_addr;

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
 * @brief Writes a block of sequential registers.
 *
 * @param addr register address to start writing to
 * @param data pointer to the data to write
 * @param len  number of bytes to write
 * @retval HAL status, HAL_OK on a successful write
 */
static HAL_StatusTypeDef burst_write(uint8_t addr, uint8_t *data, uint16_t len);


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
    return HAL_ERROR; //checking proper function call
  }
  bma_i2c = hi2c;
  bma_addr = (uint8_t)(dev_addr << 1);
  return HAL_OK;
}

static HAL_StatusTypeDef read_register(uint8_t addr, uint8_t *buffer, uint16_t len) {
  if (bma_i2c == NULL || buffer == NULL) {
    return HAL_ERROR; //checking proper function call
  }
  return HAL_I2C_Mem_Read(bma_i2c, bma_addr, addr, I2C_MEMADD_SIZE_8BIT, buffer, len,
                          BMA580_I2C_TIMEOUT_MS); //using built-in function for reading register
}

static HAL_StatusTypeDef write_register(uint8_t addr, uint8_t data) {
  if (bma_i2c == NULL) {
    return HAL_ERROR; //checking proper function call
  }
  return HAL_I2C_Mem_Write(bma_i2c, bma_addr, addr, I2C_MEMADD_SIZE_8BIT, &data, 1,
                           BMA580_I2C_TIMEOUT_MS); //using built-in function for writing register. hardcoded to 1 byte
}

static HAL_StatusTypeDef burst_write(uint8_t addr, uint8_t *data, uint16_t len) {
  if (bma_i2c == NULL) {
    return HAL_ERROR; //checking proper function call
  }
  return HAL_I2C_Mem_Write(bma_i2c, bma_addr, addr, I2C_MEMADD_SIZE_8BIT, data, len,
                           BMA580_I2C_TIMEOUT_MS); //using built-in function for writing register block
}

static HAL_StatusTypeDef poll_register(uint8_t addr, uint8_t mask, uint8_t expected,
                                       uint32_t timeout_ms) {
  uint32_t start = HAL_GetTick(); //get tick count when function called

  for (;;) { //will read the register until the condition is met or timeout occurs
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

float getAcceleration(SPI_HandleTypeDef *hspi){
    uint8_t values[6];
    float xAcceleration, yAcceleration, zAcceleration;
    read_register(0x00, values, 0x06);

    // Get 2s complement of x, y and z axis data
    int16_t x2s = (int16_t)((values[1] << 8) | values[0]);
    int16_t y2s = (int16_t)((values[3] << 8) | values[2]);
    int16_t z2s = (int16_t)((values[5] << 8) | values[4]);

    log_record(hspi, x2s , y2s, z2s);

    //TODO send raw data to logger and convert to metric in post-processing
    
    // Convert number to right metric by multiplying by range and dividing by 2^16
    xAcceleration = (float)(x2s) * 16 / 32768;

    // Convert number to right metric by multiplying by range and dividing by 2^16
    yAcceleration = (float)(y2s) * 16 / 32768;

    // Convert number to right metric by multiplying by range and dividing by 2^16
    zAcceleration = (float)(z2s) * 16 / 32768;

    // The magnitude of the accelertion vector
    return sqrt(pow(xAcceleration, 2) + pow(yAcceleration, 2) + pow(zAcceleration, 2));
}

/*
 * TEMPORARY: blinks out why the chip ID read failed. Remove once the sensor
 * initialises reliably.
 *
 *   1 red  - no acknowledge; then 1 green per address that answers a bus scan
 *   2 red  - timeout (usually SDA/SCL held low: missing pull-ups or stuck bus);
 *            then the idle line levels, SDA first then SCL, green = high,
 *            red = low (a healthy bus is green, green)
 *   3 red  - bus error / arbitration lost (noise, wiring)
 *   4 red  - read worked but the chip ID is wrong; then the ID it got,
 *            MSB first, green = 1, red = 0 (0xC4 = G G R R R G R R)
 */
static void report_chip_id_failure(HAL_StatusTypeDef status, uint8_t chip_id) {
  uint32_t err = HAL_I2C_GetError(bma_i2c);
  uint8_t reds;

  if (status == HAL_OK) {
    reds = 4;
  } else if (err & HAL_I2C_ERROR_AF) {
    reds = 1;
  } else if (err & HAL_I2C_ERROR_TIMEOUT) {
    reds = 2;
  } else {
    reds = 3;
  }

  for (uint8_t i = 0; i < reds; i++) {
    flashLED(1);
  }
  HAL_Delay(2000);

  if (reds == 1) {
    for (uint16_t addr = 0x08; addr < 0x78; addr++) {
      if (HAL_I2C_IsDeviceReady(bma_i2c, (uint16_t)(addr << 1), 2, 5) == HAL_OK) {
        flashLED(2);
      }
    }
  } else if (reds == 2) {
    /* The input register still reflects the pin level in alternate-function mode. */
    flashLED(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET ? 2 : 1); // SDA
    flashLED(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_SET ? 2 : 1); // SCL
  } else if (reds == 4) {
    for (int bit = 7; bit >= 0; bit--) {
      flashLED(((chip_id >> bit) & 1U) ? 2 : 1);
    }
  }
}

HAL_StatusTypeDef BMA580_Init(void) {
  HAL_StatusTypeDef status;
  uint8_t chip_id;

  HAL_Delay(BMA580_POWER_ON_DELAY_MS);

  (void)write_register(0x7E, 0xB6);  // soft reset
  HAL_Delay(3);                      

  /*
   * The first transaction after power-up selects I2C as the interface. The
   * sensor NACKs it by design (datasheet 5.2.1.1), so its status is ignored.
   */
  (void)read_register(0x00, &chip_id, 1);

  status = read_register(0x00, &chip_id, 1);
  if (status != HAL_OK || chip_id != BMA580_CHIP_ID) {
    return (status != HAL_OK) ? status : HAL_ERROR;
  }

  // Leave SUSPEND (the power-on default) for ACTIVE
  status = write_register(0x04, 0x00);
  if (status != HAL_OK) {
    return status;
  }
  /* A write issued in SUSPEND needs 450 us of bus idle before the next access. */
  HAL_Delay(1);

  // Wait for HEALTH_STATUS.sensor_health_status to report a good state
  status = poll_register(0x02, 0x0F, 0x0F, BMA580_STATUS_TIMEOUT_MS);
  if (status != HAL_OK) {
    return status;
  }

  // INT2 shares pin 3 with SPI CSB, so in I2C mode it must be an output
  // (datasheet 5.2.1.1). Push-pull, active low: idles driven high. Nothing is mapped to it.
  status = write_register(0x35, 0x01);
  if (status != HAL_OK) {
    return status;
  }

  // Configure Sensor Parameters
  status = write_register(0x30, 0b00000000); // disable accelerometer to change config
  if (status != HAL_OK) {
    return status;
  }

  status = write_register(0x31, 0b00101000); // ODR 400Hz, Average 4 samples, Low Power Mode //TODO see if we should decrease Hz
  if (status != HAL_OK) {
    return status;
  }

  status = write_register(0x32, 0b00001111); // +/- 16g, -60dB roll-off, low noise mode, status flag of acc_drdy_int is not cleared automatically.
  if (status != HAL_OK) {
    return status;
  }

  status = write_register(0x34, 0b00000101); // INT1 (wired to PB0): latched, open drain, active low
  if (status != HAL_OK) {
    return status;
  }

  status = write_register(0x30, 0b00001111); // Enable accelerometer and temp sensor
  if (status != HAL_OK) {
    return status;
  }

  // Writing to extended register map to set GENERIC_INTERRUPT1_1 to correct settings
  uint8_t data[6];
  data[0] = 0b00000000; //low byte 0x04
  data[1] = 0b11100101; //high byte 0x04 1.5g any axis (revert to 0b11101010 for 5g any axis)

  data[2] = 0b00000100; //low 0x05
  data[3] = 0b00010100; //high 0x05 default hysteresis. acc_ref_up set to manual

  data[4] = 0b00110010; //low (revert to 01001011 for 1.5 seconds)
  data[5] = 0b11100000; //high 0x06 wait time 0.14s duration 1.5s

  // Wait for FEAT_ENG_GP_FLAGS.feat_init_stat to report the feature engine initialised
  status = poll_register(0x52, 0x03, 0x01, BMA580_STATUS_TIMEOUT_MS);
  if (status != HAL_OK) {
    return status;
  }

  write_register(0x5E, 0x04);
  status = burst_write(0x5F, data, 6);
  if (status != HAL_OK) {
    flashLED(1); // Flash Red to indicate initialization failure
    return status;
  }

  status = write_register(0x36, 0b01000000); // map generic interrupt 1 to INT1 pin
  if (status != HAL_OK) {
    return status;
  }
  status = write_register(0x55, 0x01);   // enable generic interrupt 1 FEAT_ENG_GPR_0.gen_int1_en = 1
  if (status != HAL_OK) {
    return status;
  }
  status = write_register(0x54, 0x01);   // enable generic interrupt 1 FEAT_ENG_GPR_CTRL.update_gprs = 1 (required or the enable is ignored)
  if (status != HAL_OK) {
    return status;
  }


  return poll_register(0x11, 0x01, 0x01, BMA580_STATUS_TIMEOUT_MS);
}

// This function gets called by the interrupt
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == BMA580_INT_Pin) {
    bma_interrupt = 1;
  }
}

bool launched_interrupt(void) {
  if (bma_interrupt == 0) {
    return false;
  }
  bma_interrupt = 0;
  return true;
}