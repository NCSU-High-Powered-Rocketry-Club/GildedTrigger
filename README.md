# Launch Relay Board

Bare-metal firmware for the NC State High-Powered Rocketry Club's launch relay board, the payload release controller (repo name: GildedTrigger). It detects launch and landing with an accelerometer, then fires the release switches once the rocket is on the ground. Flying on the club's IREC 2027 competition rocket.

## Hardware

| Part | Role | Interface |
| --- | --- | --- |
| STM32U083KC (Arm Cortex-M0+) | Main microcontroller | — |
| Bosch BMA580 | 3-axis accelerometer, ±16 g, 400 Hz | I2C2 (PA6 SDA, PA7 SCL), INT on PB0 |
| Macronix MX25R3235F | 4 MB NOR flash for the flight log | SPI3 (PB3–PB5), CS on PB6 |
| SWITCH1 / SWITCH2 | Payload release outputs | GPIO PA1, PA2 |
| LED1 / LED2 | Status indicators | GPIO PA12, PA11 |
| Debug header | Programming and debugging | SWD (PA13 SWDIO, PA14 SWCLK) |

Also wired but not yet used in firmware: battery voltage sense (ADC1 on PA3), a log dump input (PA8), and USART1 at 115200 baud (PA9/PA10).

## How it works

1. **Startup.** The MCU initializes its peripherals and configures the BMA580: 400 Hz output, ±16 g range, low-power mode, and a motion interrupt that fires above 5 g on any axis. If the sensor fails to initialize, LED1 flashes.
2. **Wait for launch.** The MCU enters sleep mode. Liftoff acceleration trips the BMA580's interrupt, which pulls PB0 low and wakes the MCU through an EXTI falling-edge interrupt.
3. **Detect landing.** After launch, the main loop reads the accelerometer continuously and computes the magnitude of the acceleration vector. When the magnitude stays within 0.1 g of 1 g for 1 second straight, the rocket is considered landed.
4. **Release the payload.** The firmware drives SWITCH1 and SWITCH2 high for 5 ms to trigger the release.
5. **Log everything.** Every accelerometer reading after launch is written to flash (see below) for post-flight analysis.

## Source layout

All firmware is in `PayloadSwitch/Core/`.

| File | Purpose |
| --- | --- |
| `Src/main.c` | Peripheral setup, sleep until launch, main loop |
| `Src/bma580.c` | I2C driver for the BMA580: register reads and writes, init sequence, interrupt handling |
| `Src/trigger.c` | Landing detection and release switch control |
| `Src/flash_memory_driver.c` | SPI driver for the MX25R3235F: page program, chip erase, status polling, deep sleep |
| `Src/flight_log.c` | Buffers records and writes them to flash one page at a time |
| `Src/led.c` | Status LED helper |

## Flight log format

Each reading is stored as an 11-byte packed, little-endian record:

| Offset | Size | Field | Notes |
| --- | --- | --- | --- |
| 0 | 1 | `sync` | Always `0xA5` |
| 1 | 4 | `timestamp` | Milliseconds since boot |
| 5 | 2 | `ax` | Raw signed 16-bit, ±16 g full scale |
| 7 | 2 | `ay` | Same |
| 9 | 2 | `az` | Same |

Records are buffered in RAM and written 23 at a time into 256-byte flash pages. The last 3 bytes of each page are left erased (`0xFF`). To convert a raw value to g: `value * 16 / 32768`.

## Tools

Python scripts in `PayloadSwitch/Python/`:

- `make_test_log.py` creates a synthetic `test_log.bin` in the same format as a real flash dump, for testing the decoder.
- `decoder.py` reads a flash dump, checks each record's sync byte, converts readings to g, and writes a CSV with `time, ax, ay, az`. Edit the input and output paths at the top of the script before running.

## Building and flashing

1. Open the `PayloadSwitch` folder as a project in STM32CubeIDE.
2. Build the project.
3. Connect an ST-Link to the board's SWD header and click Run

The pin and clock configuration lives in `PayloadSwitch.ioc` and can be edited with STM32CubeMX.

