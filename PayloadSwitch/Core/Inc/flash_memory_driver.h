#pragma once

#include <stdbool.h>
#include "main.h"

#define MX25R3235F_ERASE_TIMEOUT 120000
#define MX25R3235F_PAGE_TIMEOUT 10
#define CMD_READ 0x03
#define CMD_PAGEPROGRAM 0x02
#define CMD_ERASE 0xC7
#define CMD_WRITE_ENABLE 0x06
#define CMD_RDSR 0x05
#define CMD_DEEPSLEEP 0xB9
