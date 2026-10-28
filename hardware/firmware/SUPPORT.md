# Supported Hardware & Flashing Guide Matrix

> **IoT RFID Attendance Terminal Firmware**  
> **Architecture:** Decoupled C++17 with Dependency Injection & Hardware Abstraction  
> **Supported Microcontroller Families:** Espressif ESP32 (Classic, S3, C3) & STMicroelectronics STM32 (F4, F7, G4, L4, F1)  
> **Code Status:** Strictly Read-Only / Verified

---

## Quick Navigation / Table of Contents

- [1. Verification Matrix & Resource Usage](#1-verification-matrix--resource-usage)
- [2. ESP32 Family Guides](#2-esp32-family-guides)
  - [2.1 ESP32 Classic (WROVER-E / WROOM / DevKit)](#21-esp32-classic-wrover-e--wroom--devkit)
  - [2.2 ESP32-S3 (DevKitC-1 / XIAO S3)](#22-esp32-s3-devkitc-1--xiao-s3)
  - [2.3 ESP32-C3 (DevKitM-1 / XIAO C3)](#23-esp32-c3-devkitm-1--xiao-c3)
- [3. STM32 Family Guides](#3-stm32-family-guides)
  - [3.1 STM32F401 / F411 (WeAct Black Pill V3.0)](#31-stm32f401--f411-weact-black-pill-v30)
  - [3.2 STM32F446RE (ST Nucleo-64)](#32-stm32f446re-st-nucleo-64)
  - [3.3 STM32G474RE (ST Nucleo-64 Modern)](#33-stm32g474re-st-nucleo-64-modern)
  - [3.4 STM32L476RG (ST Nucleo-64 Ultra-Low Power)](#34-stm32l476rg-st-nucleo-64-ultra-low-power)
  - [3.5 STM32F746ZG (ST Nucleo-144 High-End)](#35-stm32f746zg-st-nucleo-144-high-end)
  - [3.6 STM32F103C8 (Blue Pill)](#36-stm32f103c8-blue-pill)
- [4. Global Troubleshooting & Attendly PWA Sync](#4-global-troubleshooting--attendly-pwa-sync)

---

## 1. Verification Matrix & Resource Usage

Every configuration is pre-configured in [`hardware/firmware/platformio.ini`](file:///c:/Users/1sree/Documents/attendance-esp32/hardware/firmware/platformio.ini) and verified with PlatformIO Core.

| Environment | Board Profile | Core Architecture | Clock | Flash Used | RAM Used | Verification Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `esp32dev` | **ESP32-WROVER-E / DevKit** | Xtensa Dual-Core LX6 | 240 MHz | **71.1%** (931.6 KB / 1.31 MB) | **14.5%** (47.6 KB / 327.6 KB) | ✅ **SUCCESS (0 errors)** |
| `esp32_s3` | **ESP32-S3-DevKitC-1** | Xtensa Dual-Core LX7 | 240 MHz | **26.9%** (898.0 KB / 3.34 MB) | **14.5%** (47.3 KB / 327.6 KB) | ✅ **SUCCESS (0 errors)** |
| `esp32_c3` | **ESP32-C3-DevKitM-1** | 32-bit RISC-V | 160 MHz | **71.0%** (931.0 KB / 1.31 MB) | **12.7%** (41.4 KB / 327.6 KB) | ✅ **SUCCESS (0 errors)** |
| `stm32_blackpill` | **WeAct BlackPill V3.0 (F401CE)** | ARM Cortex-M4 | 84 MHz | **17.9%** (93.8 KB / 524.2 KB) | **7.8%** (7.6 KB / 98.3 KB) | ✅ **SUCCESS (0 errors)** |
| `stm32_nucleo_f446re`| **ST Nucleo-F446RE** | ARM Cortex-M4 | 180 MHz | **16.6%** (86.9 KB / 524.2 KB) | **3.2%** (4.2 KB / 131.0 KB) | ✅ **SUCCESS (0 errors)** |
| `stm32_nucleo_g474re`| **ST Nucleo-G474RE** | ARM Cortex-M4 | 170 MHz | **17.1%** (89.8 KB / 524.2 KB) | **3.4%** (4.4 KB / 131.0 KB) | ✅ **SUCCESS (0 errors)** |
| `stm32_nucleo_l476rg`| **ST Nucleo-L476RG** | ARM Cortex-M4 (ULP) | 80 MHz | **8.7%** (91.2 KB / 1.04 MB) | **4.5%** (4.4 KB / 98.3 KB) | ✅ **SUCCESS (0 errors)** |
| `stm32_nucleo_f746zg`| **ST Nucleo-F746ZG** | ARM Cortex-M7 | 216 MHz | **8.8%** (92.4 KB / 1.04 MB) | **1.3%** (4.4 KB / 327.6 KB) | ✅ **SUCCESS (0 errors)** |

---

## 2. ESP32 Family Guides

### 2.1 ESP32 Classic (WROVER-E / WROOM / DevKit)
* **Environment:** `esp32dev`
* **Features:** Full standalone Wi-Fi HTTP Dashboard on port 80 + LittleFS internal flash persistence + Web Serial CLI.

#### Hardware Connections:
| Peripheral | Pin Name | ESP32 GPIO | Description / Notes |
| :--- | :--- | :--- | :--- |
| **SSD1306 OLED** | SCL | `GPIO 22` | I2C Clock (3.3V, 4.7kΩ pull-up) |
| **SSD1306 OLED** | SDA | `GPIO 21` | I2C Data (3.3V, 4.7kΩ pull-up) |
| **PN532 RFID** | SCK | `GPIO 18` | SPI Clock |
| **PN532 RFID** | MISO | `GPIO 19` | SPI Master In |
| **PN532 RFID** | MOSI | `GPIO 23` | SPI Master Out |
| **PN532 RFID** | SS / CS | `GPIO 5` | Slave Select |
| **PN532 RFID** | RST | `GPIO 4` | Reset line |
| **LED Green** | Anode (+) | `GPIO 26` | 220Ω resistor to GND |
| **LED Red** | Anode (+) | `GPIO 27` | 220Ω resistor to GND |
| **Buzzer** | PWM (+) | `GPIO 25` | Passive/Active buzzer |
| **Power** | VCC / GND | `3V3` / `GND` | Power all logic strictly with 3.3V |

#### How to Connect to PC:
1. Connect via standard Micro-USB or USB-C cable to PC.
2. Confirm the USB-to-UART bridge (CP2102 or CH340) appears in Device Manager under `Ports (COM & LPT)`.

#### How to Flash:
```bash
# 1. Ensure Attendly PWA or Serial Monitor is DISCONNECTED from the COM port
# 2. Run upload command:
pio run -e esp32dev -t upload
```
* **Auto-Reset Troubleshooting:** If esptool gets stuck on `Connecting........_____.....`:
  * Press and hold the **`BOOT`** button.
  * When esptool prints `Writing at 0x...`, release the `BOOT` button.

---

### 2.2 ESP32-S3 (DevKitC-1 / XIAO S3)
* **Environment:** `esp32_s3`
* **Features:** Native on-chip USB CDC (direct browser Web Serial without external chips) + Dual-Core 240MHz + Wi-Fi.

#### Hardware Connections:
| Peripheral | Pin Name | ESP32-S3 GPIO | Description / Notes |
| :--- | :--- | :--- | :--- |
| **SSD1306 OLED** | SCL / SDA | `GPIO 9` / `GPIO 8` | I2C Bus |
| **PN532 RFID** | SCK / MISO | `GPIO 12` / `GPIO 13` | SPI Bus |
| **PN532 RFID** | MOSI / CS | `GPIO 11` / `GPIO 10` | SPI Bus |
| **PN532 RFID** | RST | `GPIO 14` | Reset line |
| **Status LEDs** | Green / Red | `GPIO 1` / `GPIO 2` | Current limiting resistors |
| **Buzzer** | Tone PWM | `GPIO 42` | Native LEDC PWM |

#### How to Connect to PC:
* Connect your USB-C cable to the **USB port** labeled **USB** (not the UART port) to utilize the native on-chip USB CDC.

#### How to Flash:
```bash
pio run -e esp32_s3 -t upload
```
* **DFU Bootloader Sequence:** If upload does not auto-start:
  1. Hold down the **`BOOT`** button (GPIO 0).
  2. Press and release the **`RST`** button.
  3. Release the **`BOOT`** button. The chip is now in ROM download mode.
  4. Run `pio run -e esp32_s3 -t upload`.

---

### 2.3 ESP32-C3 (DevKitM-1 / XIAO C3)
* **Environment:** `esp32_c3`
* **Features:** RISC-V 32-bit architecture + Native USB JTAG/CDC + Low power.

#### Hardware Connections:
| Peripheral | Pin Name | ESP32-C3 GPIO | Description / Notes |
| :--- | :--- | :--- | :--- |
| **SSD1306 OLED** | SCL / SDA | `GPIO 9` / `GPIO 8` | Hardware I2C |
| **PN532 RFID** | SCK / MISO | `GPIO 4` / `GPIO 5` | Software SPI |
| **PN532 RFID** | MOSI / CS | `GPIO 6` / `GPIO 7` | Software SPI |
| **PN532 RFID** | RST | `GPIO 3` | Reset line |
| **Status LEDs** | Green / Red | `GPIO 0` / `GPIO 1` | 330Ω resistors |
| **Buzzer** | Tone PWM | `GPIO 2` | Tone output |

#### How to Flash:
```bash
pio run -e esp32_c3 -t upload
```
* **Bootloader Sequence:** Hold **`BOOT`** (GPIO 9), tap **`RST`**, release **`BOOT`**, then upload.

---

## 3. STM32 Family Guides

All STM32 boards operate in **Dedicated High-Speed USB Serial Mode** with **Dual-Mode MicroSD/In-RAM storage**.

### 3.1 STM32F401 / F411 (WeAct Black Pill V3.0)
* **Environment:** `stm32_blackpill`
* **Clock / Memory:** 84 MHz / 100 MHz Cortex-M4, 512KB Flash, 96KB–128KB RAM.

#### Hardware Connections:
| Peripheral | Pin Name | Black Pill Pin | Physical Location |
| :--- | :--- | :--- | :--- |
| **SSD1306 OLED** | SCL / SDA | `PB6` / `PB7` | Left header (I2C1) |
| **PN532 RFID** | SCK / MISO / MOSI | `PA5` / `PA6` / `PA7` | Right header (SPI1) |
| **PN532 RFID** | CS / RST | `PB0` / `PB1` | Right header |
| **MicroSD SPI Module** | CS / SCK / MISO / MOSI | `PA4` / `PA5` / `PA6` / `PA7` | Shared SPI1 with dedicated `PA4` CS |
| **Indicators** | Green / Red LED | `PA1` / `PA2` | Output GPIO |
| **Buzzer** | Tone | `PB8` | Hardware timer tone output |

#### How to Connect & Flash:

##### Method A: Direct USB-C DFU (No extra hardware needed)
1. Connect Black Pill to PC via USB Type-C.
2. Hold down **`BOOT0`** button.
3. Tap **`NRST`** button once.
4. Release **`BOOT0`**. The chip is now in internal ROM DFU mode (Device Manager: *STM32 BOOTLOADER*).
5. Flash via PlatformIO:
   ```bash
   pio run -e stm32_blackpill -t upload
   ```

##### Method B: ST-LINK v2 Dongle (Best for rapid debugging)
1. Wire 4 pins between ST-Link and Black Pill: `3V3` ➔ `3V3`, `GND` ➔ `GND`, `SWDIO` ➔ `DIO`, `SWCLK` ➔ `CLK`.
2. Run:
   ```bash
   pio run -e stm32_blackpill -t upload
   ```

---

### 3.2 STM32F446RE (ST Nucleo-64)
* **Environment:** `stm32_nucleo_f446re`
* **Clock / Memory:** 180 MHz Cortex-M4, 512KB Flash, 128KB RAM.
* **Special Advantage:** Onboard ST-LINK v2.1 chip features an automatic **USB Hardware Virtual COM Port** connected to `USART2`.

#### Hardware Connections (Morpho / Arduino Headers):
| Peripheral | Signal | Nucleo Pin (Morpho) | Arduino Header Equivalent |
| :--- | :--- | :--- | :--- |
| **SSD1306 OLED** | SCL / SDA | `PB6` / `PB7` | `D10` / `D4` (or `D15`/`D14`) |
| **PN532 RFID** | SCK / MISO / MOSI | `PA5` / `PA6` / `PA7` | `D13` / `D12` / `D11` |
| **PN532 RFID** | CS / RST | `PB0` / `PB1` | `A3` / `A4` |
| **MicroSD Module** | CS | `PA4` | `A2` |
| **Status LEDs** | Green / Red | `PA1` / `PA2` | Morpho Left |
| **Buzzer** | Tone Output | `PB8` | `D15` |

#### How to Flash:
1. Connect standard USB cable to the top **ST-LINK USB connector** (CN1).
2. The board enumerates as `ST-Link Debugger`, `Virtual COM Port`, and a USB Drive.
3. No button sequence needed:
   ```bash
   pio run -e stm32_nucleo_f446re -t upload
   ```
4. PlatformIO automatically uploads via OpenOCD/ST-Link and executes immediately.

---

### 3.3 STM32G474RE (ST Nucleo-64 Modern)
* **Environment:** `stm32_nucleo_g474re`
* **Clock / Memory:** 170 MHz Cortex-M4, 512KB Flash, 128KB RAM.
* **Onboard Debugger:** ST-LINK v3 with high-speed Virtual COM Port.

#### How to Flash:
1. Connect USB cable to CN1 (ST-LINK v3).
2. Run:
   ```bash
   pio run -e stm32_nucleo_g474re -t upload
   ```
3. Connects instantly to Attendly PWA over Web Serial at 115200 baud.

---

### 3.4 STM32L476RG (ST Nucleo-64 Ultra-Low Power)
* **Environment:** `stm32_nucleo_l476rg`
* **Clock / Memory:** 80 MHz Cortex-M4, 1MB Flash, 96KB RAM.
* **Best Use:** Battery-operated portable attendance terminals.

#### How to Flash:
```bash
pio run -e stm32_nucleo_l476rg -t upload
```

---

### 3.5 STM32F746ZG (ST Nucleo-144 High-End)
* **Environment:** `stm32_nucleo_f746zg`
* **Clock / Memory:** 216 MHz Cortex-M7, 1MB Flash, 320KB RAM.
* **Best Use:** High-volume user databases stored entirely in RAM.

#### How to Flash:
```bash
pio run -e stm32_nucleo_f746zg -t upload
```

---

### 3.6 STM32F103C8 (Blue Pill)
* **Environment:** Add `board = bluepill_f103c8` under `ststm32`.
* **Clock / Memory:** 72 MHz Cortex-M3, 64KB Flash, 20KB RAM.

#### Hardware Connections:
* OLED: `PB6` (SCL), `PB7` (SDA)
* RFID: `PA5` (SCK), `PA6` (MISO), `PA7` (MOSI), `PB0` (CS), `PB1` (RST)
* MicroSD: `PA4` (CS)

#### How to Flash:
* **Via ST-Link v2:** Connect `SWDIO`, `SWCLK`, `GND`, `3V3`.
  ```bash
  pio run -e stm32_bluepill -t upload
  ```
* **Via USB-UART Adapter:** Connect CP2102 to `PA9` (RX) / `PA10` (TX). Move `BOOT0` jumper to `1`, tap `RESET`, and upload via serial.

---

## 4. Global Troubleshooting & Attendly PWA Sync

1. **COM Port Access Denied during Upload:**
   * **Root Cause:** The browser (Attendly PWA) or another serial terminal currently holds an open lock on the COM port.
   * **Fix:** Click **Disconnect** in the PWA before running `pio run ... -t upload`. Once flashed, click **Connect** in the PWA.
2. **RFID Reader Shows "Simulation Mode" on Boot:**
   * **Root Cause:** The PN532 did not respond to SPI initialization at address `0x00` (loose wiring or incorrect CS/SCK pin).
   * **Safe Fallback:** The firmware automatically falls back to Serial Simulation mode so you can test scans by typing a hex UID (e.g. `A1B2C3D4`) into the console.
3. **Storage Fallback on STM32:**
   * If a MicroSD module is not attached on `PA4`, the firmware logs `Operating in volatile in-RAM repository mode` and allows full add/delete/logging directly in memory without crashing.
