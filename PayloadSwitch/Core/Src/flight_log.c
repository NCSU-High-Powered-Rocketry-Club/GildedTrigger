#include "flight_log.h"

#define RECORD_PACKET_SIZE 11 
#define RECORDS_PER_PAGE (uint8_t)(256 / RECORD_PACKET_SIZE)
#define TOTAL_PAGES 16384 // 4MB / 256 bytes per page
#define PAGE_SIZE 256
#define UART_PAGE_TIMEOUT_MS 100 // a 256-byte page takes ~22ms at 115200 baud



typedef struct __attribute__((packed)) { //packed means there is no extra bytes added for alignment
    uint8_t sync; // 0xA5 is convention.
    uint32_t timestamp;
    int16_t ax;
    int16_t ay;
    int16_t az;
    //TOMAYBE add checksum
} recordPacket; //exactly 11 bytes
_Static_assert(sizeof(recordPacket) == 11, "recordPacket must be 11 bytes"); //this will cause compile fail if its not 11

static uint32_t page = 0; //the current page we are writing to
static recordPacket buffer[RECORDS_PER_PAGE]; //buffer to hold records before writing to flash
static uint8_t index = 0; //the number of records currently in the buffer

static HAL_StatusTypeDef flush_log(SPI_HandleTypeDef *hspi){ //writes the buffer to flash memory
    uint32_t addr = page * 256;
    if (page >= TOTAL_PAGES){ //Chip full
        return HAL_ERROR;
    }
    HAL_StatusTypeDef status = programPage(hspi, (const uint8_t *)buffer, sizeof(buffer), addr);
    if (status != HAL_OK) return status;

    page++;
    return HAL_OK;
}

HAL_StatusTypeDef log_record(SPI_HandleTypeDef *hspi, int16_t ax, int16_t ay, int16_t az){ //makes the packet and stores it in memory
    recordPacket packet;
    packet.sync = 0xA5;
    packet.timestamp = HAL_GetTick();
    packet.ax = ax;
    packet.ay = ay;
    packet.az = az;

    if (index == RECORDS_PER_PAGE){ //redundant check. Only will happen if the buffer is full and flush_log failed. In that case, we want to try to flush again.
    HAL_StatusTypeDef status = flush_log(hspi);
    if (status != HAL_OK){
        return status;
    }
    index = 0;
    }

    buffer[index] = packet;
    index++;

    if (index == RECORDS_PER_PAGE){ //if the buffer is full, write it to flash memory
        HAL_StatusTypeDef status = flush_log(hspi);
        if (status != HAL_OK){
            return status;
        }
        index = 0;
    }

    return HAL_OK;

}

HAL_StatusTypeDef dump_log(SPI_HandleTypeDef *hspi, UART_HandleTypeDef *huart){ //sends every written page out over UART
    static uint8_t page_buf[PAGE_SIZE]; //static so the 256 bytes don't come off the stack

    for (uint32_t p = 0; p < TOTAL_PAGES; p++){
        HAL_StatusTypeDef status = flashRead(hspi, page_buf, PAGE_SIZE, p * PAGE_SIZE);
        if (status != HAL_OK) return status;

        if (page_buf[0] == 0xFF){ //erased flash reads 0xFF, so this page was never written. End of log
            break;
        }

        status = HAL_UART_Transmit(huart, page_buf, PAGE_SIZE, UART_PAGE_TIMEOUT_MS);
        if (status != HAL_OK) return status;
    }

    return HAL_OK;
}
