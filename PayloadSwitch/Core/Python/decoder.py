import time
import csv
file_path = "GildedTrigger\\PayloadSwitch\\Core\\Python\\test_log.bin"

with open(file_path, 'rb') as file:
    writer = csv.writer(open("GildedTrigger\\PayloadSwitch\\Core\\Python\\test_log.csv", 'w', newline=''))
    writer.writerow(["time", "ax", "ay", "az"])  # Write the header row
    for page in range(0, 16384, 1):
        page_data = file.read(256)

        if len(page_data) < 256 or page_data == b'\xff' * 256: #if the whole page is FF FF FF then end of log
            print(f"end of log at page {page}")
            break

        for pos in range(0, 256, 11):
            print(pos)
            line_data = page_data[pos:pos+11]

            if line_data[0] != 0xa5:
                print(f"invalid sync byte at page {page}, pos {pos}")
                print(line_data.hex(" "))
                continue
            
            runtime = int.from_bytes(line_data[1:5], byteorder='little')  # Extract the 10 bytes after the sync byte
            ax = int.from_bytes(line_data[5:7], byteorder='little', signed=True)  # Extract the next 2 bytes for ax
            ay = int.from_bytes(line_data[7:9], byteorder='little', signed=True)  # Extract the next 2 bytes for ay
            az = int.from_bytes(line_data[9:11], byteorder='little', signed=True)  # Extract the next 2 bytes for az
            print(line_data.hex(" "))
            print(f"time: {runtime}, ax: {ax}, ay: {ay}, az: {az}")
            writer.writerow([runtime, ax, ay, az])  # Write the data to the CSV file
            time.sleep(0.2)  # Add a delay of 0.2 seconds between each line
            # Process the line_data as needed
    file.close()
