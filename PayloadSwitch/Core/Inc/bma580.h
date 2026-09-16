/*
 * bma580.h
 *
 * Driver for the Bosch BMA580 accelerometer over I2C.
 */

#pragma once

#include <stdint.h>
#include "stm32u0xx_hal.h"

/*
 * 7-bit I2C slave addresses. The BMA580 selects between them with the SDO pin:
 * SDO tied low -> 0x18 (default), SDO tied high -> 0x19.
 * These are the raw 7-bit values; bma580_init shifts them for the HAL.
 */
#define BMA580_I2C_ADDR_LOW   0x18U
#define BMA580_I2C_ADDR_HIGH  0x19U

/**
 * @brief Binds this driver to an I2C peripheral and slave address.
 *
 * Must be called before anything else in this driver. The handle must
 * already be initialised (e.g. by MX_I2C2_Init).
 *
 * @param hi2c     initialised I2C handle the sensor is wired to
 * @param dev_addr 7-bit slave address, BMA580_I2C_ADDR_LOW or _HIGH
 * @retval HAL_OK on success, HAL_ERROR if hi2c is NULL
 */
HAL_StatusTypeDef setup_bma580(I2C_HandleTypeDef *hi2c, uint8_t dev_addr);

/**
 * @brief Brings the sensor up and applies the flight configuration.
 *
 * Call after bma580_init. Leaves the accelerometer and temperature sensor
 * enabled in low-power mode at 400 Hz and +/- 16g.
 *
 * @retval HAL_OK on success, HAL_TIMEOUT if the sensor never reports ready,
 *         otherwise the status of the I2C transfer that failed
 */
HAL_StatusTypeDef BMA580_Init(void);
