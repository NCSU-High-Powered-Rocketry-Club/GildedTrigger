import serial

numBytes = 4096
#change COM7 to your serial port found in device manager
s = serial.Serial("COM7", 115200, timeout=3)

with open("flight.bin", "wb") as f:
    while (chunk := s.read(numBytes)):
        f.write(chunk)