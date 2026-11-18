import os

import serial

numBytes = 4096
DUMP_COMMAND = b"D"  # must match DUMP_COMMAND in main.c
ERASE_COMMAND = b"E"  # must match ERASE_COMMAND in main.c

# Save next to this script, which is where decoder.py looks for it
output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "flight.bin")

#change COM7 to your serial port found in device manager
s = serial.Serial("COM7", 115200, timeout=3)

s.reset_input_buffer()  # throw away anything that arrived before we asked
s.write(DUMP_COMMAND)   # tell the board to start sending the log

total = 0
with open(output_path, "wb") as f:
    # The board sends pages back to back, so 3 seconds of silence means it's finished
    while (chunk := s.read(numBytes)):
        f.write(chunk)
        total += len(chunk)
        print(f"received {total} bytes ({total // 256} pages)", end="\r")

s.close()
print(f"\nsaved {total} bytes to {output_path}")
if total == 0:
    print("nothing received - check wiring, COM port, and that the board was power cycled since its last flight")
