import struct

# Mirrors flight_log.c: 11 byte packed records, 23 per page (253 bytes), the
# last 3 bytes of each 256 byte page are never programmed so they stay 0xFF.
# STM32 is little endian, so the packet is '<BIhhh'.
RECORD_FORMAT = '<BIhhh'
RECORD_PACKET_SIZE = struct.calcsize(RECORD_FORMAT)  # 11
PAGE_SIZE = 256
RECORDS_PER_PAGE = PAGE_SIZE // RECORD_PACKET_SIZE  # 23
SYNC = 0xA5

FULL_PAGES = 4    # pages of real records
ERASED_PAGES = 2  # blank (0xFF) pages after the log, like an unwritten chip

out_path = "test_log.bin"

data = bytearray()
n = 0
for page in range(FULL_PAGES):
    page_data = bytearray()
    for _ in range(RECORDS_PER_PAGE):
        timestamp = 1000 + n * 10  # HAL_GetTick() in ms, 100 Hz logging
        ax = n
        ay = -n
        az = 2048
        # A few records hit the int16 limits to check signed decoding
        if n == 30:
            ax, ay, az = 32767, -32768, -1
        page_data += struct.pack(RECORD_FORMAT, SYNC, timestamp, ax, ay, az)
        n += 1
    page_data += b'\xFF' * (PAGE_SIZE - len(page_data))
    data += page_data

data += b'\xFF' * (PAGE_SIZE * ERASED_PAGES)

with open(out_path, 'wb') as f:
    f.write(data)

print(f"Wrote {out_path}: {len(data)} bytes, {n} records, "
      f"{FULL_PAGES} full pages + {ERASED_PAGES} erased pages")
