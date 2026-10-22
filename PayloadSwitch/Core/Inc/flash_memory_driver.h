#pragma once

#include <stdbool.h>
#include "main.h"

#define MX25R3235F_ERASE_TIMEOUT 120000
#define MX25R3235F_PAGE_TIMEOUT 20
#define CMD_READ 0x03
#define CMD_PAGEPROGRAM 0x02
#define CMD_ERASE 0xC7
#define CMD_WRITE_ENABLE 0x06
#define CMD_RDSR 0x05
#define CMD_DEEPSLEEP 0xB9

/** 
 * @brief Erases the entire flash memory.
 * @param hspi Pointer to the SPI handle
 * @retval HAL_OK if successful, HAL_ERROR if failed
*/
HAL_StatusTypeDef ChipErase(SPI_HandleTypeDef *hspi);

/** 
 * @brief Puts the flash memory in deep sleep mode.
 * @param hspi Pointer to the SPI handle
 * @retval HAL_OK if successful, HAL_ERROR if failed
*/
HAL_StatusTypeDef deepSleep(SPI_HandleTypeDef *hspi);

/** 
 * @brief Wakes up the flash memory from deep sleep mode.
 * @param hspi Pointer to the SPI handle
*/
void wakeUp(SPI_HandleTypeDef *hspi);

/** 
 * @brief Programs a page in the flash memory. pages are 256 bytes long.
 *        If the data to be written crosses a page boundary, the function will return an error
 * @param hspi Pointer to the SPI handle
 * @param pdata Pointer to the data to be written 
 * @param len Length of the data to be written
 * @param addr Address where the data should be written
 * @retval HAL_OK if successful, HAL_ERROR if failed
*/
HAL_StatusTypeDef programPage(SPI_HandleTypeDef *hspi, const uint8_t *pdata, int len, uint32_t addr);


