import serial

s = serial.Serial("COM5", 115200, timeout=3)

with open("flight.bin", "wb") as f:
    while (chunk := s.read(4096)):
        f.write(chunk)