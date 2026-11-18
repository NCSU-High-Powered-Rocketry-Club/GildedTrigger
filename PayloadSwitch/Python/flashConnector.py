import argparse
import os

import serial

numBytes = 4096
DUMP_COMMAND = b"D"  # must match DUMP_COMMAND in main.c
ERASE_COMMAND = b"E"  # must match ERASE_COMMAND in main.c
ERASE_OK = b"K"  # must match ERASE_OK in main.c
ERASE_TIMEOUT_S = 150  # chip erase can take up to 120 s (MX25R3235F_ERASE_TIMEOUT), plus margin

# Save next to this script, which is where decoder.py looks for it
output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "flight.bin")


def dump(s):
    s.reset_input_buffer()  # throw away anything that arrived before we asked
    s.write(DUMP_COMMAND)   # tell the board to start sending the log

    total = 0
    with open(output_path, "wb") as f:
        # The board sends pages back to back, so 3 seconds of silence means it's finished
        while (chunk := s.read(numBytes)):
            f.write(chunk)
            total += len(chunk)
            print(f"received {total} bytes ({total // 256} pages)", end="\r")

    print(f"\nsaved {total} bytes to {output_path}")
    if total == 0:
        print("nothing received - check wiring, COM port, and that the board was power cycled since its last flight")


def erase(s):
    answer = input("This erases ALL flight data on the board. Dump it first if you need it. Continue? [y/N] ")
    if answer.strip().lower() != "y":
        print("erase cancelled")
        return

    s.reset_input_buffer()
    s.write(ERASE_COMMAND)
    print(f"erasing, this can take up to {ERASE_TIMEOUT_S} s...")

    s.timeout = ERASE_TIMEOUT_S  # the board only replies once the erase is done
    reply = s.read(1)

    if reply == ERASE_OK:
        print("erase complete")
    elif reply == b"":
        print("no reply - check wiring, COM port, and that the board was power cycled since its last flight")
    else:
        print(f"erase failed (board replied {reply!r})")


parser = argparse.ArgumentParser(description="Dump or erase the flight log on the board over UART.")
action = parser.add_mutually_exclusive_group(required=True)
action.add_argument("-d", "--dump", action="store_true", help="download the flight log to flight.bin")
action.add_argument("-e", "--erase", action="store_true", help="erase the whole flash chip")
#change the default to your serial port found in device manager
parser.add_argument("-p", "--port", default="COM7", help="serial port of the USB to TTL bridge (default: COM7)")
args = parser.parse_args()

s = serial.Serial(args.port, 115200, timeout=3)
try:
    if args.dump:
        dump(s)
    else:
        erase(s)
finally:
    s.close()
