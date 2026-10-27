import csv


TOTAL_PAGES = 16384
BYTES_PER_PAGE = 256
STRUCT_SIZE = 11  # 1 byte sync + 4 bytes timestamp + 3 * 2 bytes for ax, ay, az
input_file_name = "test_log.bin"
output_file_name = "test_log1.csv"


output_file_path = f"GildedTrigger\\PayloadSwitch\\Python\\{output_file_name}"
input_file_path = f"GildedTrigger\\PayloadSwitch\\Python\\{input_file_name}"
with open(input_file_path, 'rb') as file:
    writer = csv.writer(open(output_file_path, 'w', newline=''))
    writer.writerow(["time", "ax", "ay", "az"])  # Write the header row
    
    for page in range(0, TOTAL_PAGES, 1):
        page_data = file.read(BYTES_PER_PAGE)

        if len(page_data) < BYTES_PER_PAGE or page_data == b'\xff' * BYTES_PER_PAGE: #if the whole page is FF FF FF then end of log
            print(f"end of log at page {page}")
            break

        for pos in range(0, BYTES_PER_PAGE - STRUCT_SIZE + 1, STRUCT_SIZE):
            line_data = page_data[pos:pos+STRUCT_SIZE]

            if line_data[0] != 0xa5:
                print(f"invalid sync byte at page {page}, pos {pos}")
                print(line_data.hex(" "))
                continue
            
            runtime = int.from_bytes(line_data[1:5], byteorder='little')  # Extract the 10 bytes after the sync byte
            ax = int.from_bytes(line_data[5:7], byteorder='little', signed=True)  # Extract the next 2 bytes for ax
            ay = int.from_bytes(line_data[7:9], byteorder='little', signed=True)  # Extract the next 2 bytes for ay
            az = int.from_bytes(line_data[9:11], byteorder='little', signed=True)  # Extract the next 2 bytes for az
            
            ax = ax * 16 / 32768  #convert all accel values to g's
            ay = ay * 16 / 32768  #16g is the defined range of the accelerometer, 32768 is the max value of a signed int16
            az = az * 16 / 32768

            #print(f"time: {runtime}, ax: {ax}, ay: {ay}, az: {az}")
            writer.writerow([runtime, ax, ay, az])  # Write the data to the CSV file

