#include "flash_memory_driver.h"


/**
 * @brief Logs a flight record with the given accelerometer data.
 *
 * @param hspi Pointer to the SPI handle.
 * @param ax Acceleration along the x-axis.
 * @param ay Acceleration along the y-axis.
 * @param az Acceleration along the z-axis.
 * @return HAL_StatusTypeDef Status of the operation.
 */
HAL_StatusTypeDef log_record(SPI_HandleTypeDef *hspi, int16_t ax, int16_t ay, int16_t az);
