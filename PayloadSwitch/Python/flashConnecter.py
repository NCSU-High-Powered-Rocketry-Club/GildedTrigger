import serial

# Baud rate:  300 bps to 1 Mbps
s = serial.Serial("COM4", 300, timeout=3)

with open("flight.bin", "wb") as f:
    while (chunk := s.read(4096)):
        f.write(chunk)