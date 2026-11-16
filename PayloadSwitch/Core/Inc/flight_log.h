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

/**
 * @brief Sends the logged pages from flash out over UART as raw bytes.
 *
 * Sends whole 256-byte pages starting at page 0, and stops at the first
 * erased page (first byte 0xFF), which marks the end of the log.
 *
 * @param hspi Pointer to the SPI handle the flash is on.
 * @param huart Pointer to the UART handle to send the data on.
 * @return HAL_StatusTypeDef Status of the operation.
 */
HAL_StatusTypeDef dump_log(SPI_HandleTypeDef *hspi, UART_HandleTypeDef *huart);
