/*

#define RECORD_PACKET_SIZE 11 
#define RECORDS_PER_PAGE (uint8_t)(256 / RECORD_PACKET_SIZE)
#define TOTAL_PAGES 16384 // 4MB / 256 bytes per page



typedef struct __attribute__((packed)) { //packed means there is no extra bytes added for alignment
    uint8_t sync; // 0xA5 is convention.
    uint32_t timestamp;
    int16_t ax;
    int16_t ay;
    int16_t az;
    //TOMAYBE add checksum
} recordPacket; //exactly 11 bytes
// _Static_assert(sizeof(recordPacket) == 11, "recordPacket must be 11 bytes"); //this will cause compile fail if its not 11

static uint32_t page = 0; //the current page we are writing to
static recordPacket buffer[RECORDS_PER_PAGE]; //buffer to hold records before writing to flash
static uint8_t count = 0; //the number of records currently in the buffer

static HAL_StatusTypeDef flush_log(){
    uint32_t addr = page * 256;
    pageProgram(buffer, addr)
    if chip full: error
    page++;
    return HAL_OK;
}

HAL_StatusTypeDef log_record(int16_t ax, int16_t ay, int16_t az){ //makes the packet and stores it in memory
    recordPacket packet;
    // fill out the packet

    //if buffer is full, flush to flash. count = 0
    //add the packet to buffer. count++

}


*/