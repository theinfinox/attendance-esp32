# 📡 Attendly Terminal Firmware — Universal ESP32 & STM32 RFID Platform

<p align="center">
  <img src="https://img.shields.io/badge/PlatformIO-Core%20v6.1%2B-orange?logo=platformio&style=for-the-badge" alt="PlatformIO" />
  <img src="https://img.shields.io/badge/C%2B%2B-17%20Standard-blue?logo=c%2B%2B&style=for-the-badge" alt="C++17" />
  <img src="https://img.shields.io/badge/ESP32-Classic%20%7C%20S3%20%7C%20C3-red?logo=espressif&style=for-the-badge" alt="ESP32" />
  <img src="https://img.shields.io/badge/STM32-F1%20%7C%20F4%20%7C%20F7%20%7C%20G4%20%7C%20L4-00205B?logo=stmicroelectronics&style=for-the-badge" alt="STM32" />
  <img src="https://img.shields.io/badge/Web%20Serial-Chromium%20PWA-green?logo=googlechrome&style=for-the-badge" alt="Web Serial" />
  <img src="https://img.shields.io/badge/Build%20Status-All%2010%20Passing-success?style=for-the-badge" alt="Build Status" />
</p>

<p align="center">
  <a href="https://serial-attendance-pwa.vercel.app/" target="_blank">
    <img src="https://img.shields.io/badge/Live%20Testing%20App-serial--attendance--pwa.vercel.app-brightgreen?style=for-the-badge&logo=vercel" alt="Live Testing Web App" />
  </a>
</p>

---

## 📖 Overview & Project Scope

**Attendly Terminal Firmware** is an **educational and foundational base project** designed to demonstrate how an IoT RFID attendance terminal works from the ground up. It serves as an extensible starting architecture for developers, makers, and engineers looking to understand embedded hardware workflows or build their own customized attendance management hardware.

> [!NOTE]
> **Foundational Base Project (Not a Turnkey Commercial End-Product):**  
> While the codebase is engineered with **robust, production-grade C++17 design patterns** (Dependency Injection, strict Hardware Abstraction, non-blocking state machine, and automatic fault-tolerance), the project itself is intended as a **reference base template and learning sandbox**—not a finished commercial product. Use it as a solid launchpad to understand, customize, and expand into your own specialized hardware solutions.

A single codebase compiles out-of-the-box for **10 distinct microcontroller targets** spanning the **Espressif ESP32** and **STMicroelectronics STM32** families:

- **Chromium Web Serial Interface:** Bi-directional sync with the live hosted Attendly testing PWA ([serial-attendance-pwa.vercel.app](https://serial-attendance-pwa.vercel.app/)) over native USB CDC or hardware UART at **115200 baud (8N1)**.
- **Dual-Mode Persistence:** LittleFS flash storage on ESP32, and MicroSD card SPI storage with transparent in-RAM volatile fallback on STM32.
- **Zero-Code Switching:** Switch between microcontrollers simply by passing the `-e <target>` flag to PlatformIO.
- **Hardware Fault-Tolerance:** Automatic graceful degradation to RFID Simulation Mode and In-RAM repository if external modules are absent during bench testing.

---

## 📑 Table of Contents

1. [🛠️ Pre-Flashing Preparation & Checklist](#1-pre-flashing-preparation--checklist)
   - [Host Toolchain Installation](#host-toolchain-installation)
   - [USB Drivers & Bootloader Utilities](#usb-drivers--bootloader-utilities)
   - [COM Port Isolation & Attendly Sync](#com-port-isolation--attendly-sync)
   - [Electrical & Logic Level Rules](#electrical--logic-level-rules)
2. [⚡ 1-Minute Quick Start](#2-1-minute-quick-start)
3. [📊 Master Verification & Resource Usage Matrix](#3-master-verification--resource-usage-matrix)
4. [🟦 ESP32 Family Modular Guides](#4-esp32-family-modular-guides)
   - [4.1 ESP32 Classic (DevKit / WROVER / WROOM)](#41-esp32-classic-wrover-e--wroom--devkit)
   - [4.2 ESP32-S3 (DevKitC-1 / XIAO S3)](#42-esp32-s3-devkitc-1--xiao-s3)
   - [4.3 ESP32-C3 (DevKitM-1 / XIAO C3 RISC-V)](#43-esp32-c3-devkitm-1--xiao-c3-risc-v)
5. [🟪 STM32 Family Modular Guides](#5-stm32-family-modular-guides)
   - [5.1 STM32F401 / F411 (WeAct Black Pill V3.0)](#51-stm32f401--f411-weact-black-pill-v30)
   - [5.2 STM32F446RE (ST Nucleo-64)](#52-stm32f446re-st-nucleo-64)
   - [5.3 STM32G474RE (ST Nucleo-64 Modern)](#53-stm32g474re-st-nucleo-64-modern)
   - [5.4 STM32G491RE (ST Nucleo-64 Access Line)](#54-stm32g491re-st-nucleo-64-access-line)
   - [5.5 STM32L476RG (ST Nucleo-64 Ultra-Low Power)](#55-stm32l476rg-st-nucleo-64-ultra-low-power)
   - [5.6 STM32F746ZG (ST Nucleo-144 High-End)](#56-stm32f746zg-st-nucleo-144-high-end)
   - [5.7 STM32F103C8 / CB (Blue Pill 128K)](#57-stm32f103c8--cb-blue-pill-128k)
6. [🔌 Web Serial CLI & PWA Integration](#6-web-serial-cli--pwa-integration)
7. [🏗️ Software Architecture & Dependency Injection](#7-software-architecture--dependency-injection)
8. [❓ Troubleshooting & FAQ](#8-troubleshooting--faq)

---

## 1. 🛠️ Pre-Flashing Preparation & Checklist

Before plugging in your board or executing any flash commands, complete this 4-step pre-flight checklist.

### Step 1: Host Toolchain Installation
Ensure you have the PlatformIO development environment installed:
* **Option A (VS Code IDE):** Install the [PlatformIO IDE Extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) in VS Code.
* **Option B (PlatformIO Core CLI):**
  ```bash
  pip install --upgrade platformio
  pio --version  # Verify installation (v6.1+)
  ```

### Step 2: USB Drivers & Bootloader Utilities
Different boards require specific host USB drivers to enumerate properly:

| Microcontroller Target | Required Host Driver | Download / Setup Method |
| :--- | :--- | :--- |
| **ESP32 Classic (DevKit)** | Silicon Labs CP210x or WCH CH340 | [CP210x VCP](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) / [CH340 Driver](http://www.wch-ic.com/downloads/CH341SER_EXE.html) |
| **ESP32-S3 / ESP32-C3** | Native USB CDC / JTAG | Native in Windows 10/11 & Linux (no driver required) |
| **STM32 Nucleo (All)** | ST-LINK USB Virtual COM & Debugger | [STSW-LINK009 Driver](https://www.st.com/en/development-tools/stsw-link009.html) |
| **STM32 Black Pill (DFU)** | WinUSB Driver for STM32 DFU Mode | Use [Zadig](https://zadig.akeo.ie/) to assign `WinUSB` to `STM32 BOOTLOADER` |
| **STM32 Blue Pill / Standalone** | ST-LINK v2 Dongle or USB-UART | Install STSW-LINK009 or standard FTDI/CH340 drivers |

### Step 3: COM Port Isolation & Attendly Sync
> [!WARNING]
> **Avoid Port Access Conflicts!**  
> Operating systems permit only **one active application** to hold an open lock on a serial COM port at a time. If the **Attendly PWA**, Arduino Serial Monitor, or PuTTY is connected to your board's COM port, PlatformIO will fail with `Access is denied` or `Could not open port`.  
> **Always click "Disconnect" in Attendly before flashing!**

### Step 4: Electrical & Logic Level Rules
* **Logic Level:** All supported microcontrollers operate strictly at **3.3V logic**. Never supply 5V directly to ESP32 GPIOs or STM32 non-5V-tolerant pins.
* **I²C Pull-Ups:** The SSD1306 OLED display requires pull-up resistors on `SDA` and `SCL` (usually 4.7kΩ, pre-soldered on most standard breakout modules).
* **SPI Bus Sharing:** The PN532 RFID reader and MicroSD module share the same SPI bus (`SCK`, `MISO`, `MOSI`) but **MUST** have separate Chip Select (`CS`) pins.

---

## 2. ⚡ 1-Minute Quick Start

```bash
# 1. Clone the repository
git clone https://github.com/your-org/attendance-esp32.git
cd attendance-esp32/hardware/firmware

# 2. Select your target environment from the matrix below and compile:
pio run -e esp32dev                # For ESP32 Classic
pio run -e stm32_nucleo_f446re     # For STM32 Nucleo-F446RE

# 3. Connect your board and upload:
pio run -e <target_name> -t upload

# 4. Open the live testing PWA (hosted directly from this repo) in Chrome/Edge:
#    👉 https://serial-attendance-pwa.vercel.app/
#    Click "Connect" at 115200 baud!
```

---

## 3. 📊 Master Verification & Resource Usage Matrix

All 9 configurations are pre-defined in [`hardware/firmware/platformio.ini`](file:///c:/Users/1sree/Documents/attendance-esp32/hardware/firmware/platformio.ini) and verified via automated build pipelines.

| Environment Identifier | Target Board | CPU Architecture | Frequency | Flash Footprint | RAM Footprint | Build Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `esp32dev` | **ESP32-WROVER-E / DevKit** | Xtensa Dual-Core LX6 | 240 MHz | **71.1%** (931.6 KB / 1.31 MB) | **14.5%** (47.6 KB / 327.6 KB) | ✅ **SUCCESS (0 errors)** |
| `esp32_s3` | **ESP32-S3-DevKitC-1** | Xtensa Dual-Core LX7 | 240 MHz | **26.9%** (898.0 KB / 3.34 MB) | **14.5%** (47.3 KB / 327.6 KB) | ✅ **SUCCESS (0 errors)** |
| `esp32_c3` | **ESP32-C3-DevKitM-1** | 32-bit RISC-V | 160 MHz | **71.0%** (931.0 KB / 1.31 MB) | **12.7%** (41.4 KB / 327.6 KB) | ✅ **SUCCESS (0 errors)** |
| `stm32_blackpill` | **WeAct BlackPill V3.0 (F401CE)** | ARM Cortex-M4 + FPU | 84 MHz | **17.9%** (93.8 KB / 524.2 KB) | **7.8%** (7.6 KB / 98.3 KB) | ✅ **SUCCESS (0 errors)** |
| `stm32_nucleo_f446re`| **ST Nucleo-F446RE** | ARM Cortex-M4 + FPU | 180 MHz | **16.6%** (86.9 KB / 524.2 KB) | **3.2%** (4.2 KB / 131.0 KB) | ✅ **SUCCESS (0 errors)** |
| `stm32_nucleo_g474re`| **ST Nucleo-G474RE** | ARM Cortex-M4 + FPU | 170 MHz | **17.8%** (93.2 KB / 524.2 KB) | **3.4%** (4.4 KB / 131.0 KB) | ✅ **SUCCESS (0 errors)** |
| `stm32_nucleo_g491re`| **ST Nucleo-G491RE** | ARM Cortex-M4 + FPU | 170 MHz | **17.7%** (92.7 KB / 524.2 KB) | **3.8%** (4.4 KB / 114.7 KB) | ✅ **SUCCESS (0 errors)** |
| `stm32_nucleo_l476rg`| **ST Nucleo-L476RG (ULP)** | ARM Cortex-M4 + FPU | 80 MHz | **8.7%** (91.4 KB / 1.04 MB) | **4.5%** (4.4 KB / 98.3 KB) | ✅ **SUCCESS (0 errors)** |
| `stm32_nucleo_f746zg`| **ST Nucleo-144 (F746ZG)** | ARM Cortex-M7 + DP-FPU | 216 MHz | **8.8%** (91.5 KB / 1.04 MB) | **1.3%** (4.4 KB / 327.6 KB) | ✅ **SUCCESS (0 errors)** |
| `stm32_bluepill` | **STM32F103C8 / CB Blue Pill** | ARM Cortex-M3 | 72 MHz | **64.0%** (83.9 KB / 131.1 KB) | **20.2%** (4.1 KB / 20.5 KB) | ✅ **SUCCESS (0 errors)** |

---

## 4. 🟦 ESP32 Family Modular Guides

### 4.1 ESP32 Classic (WROVER-E / WROOM / DevKit)

```
Target Flag:      pio run -e esp32dev -t upload
Architecture:     Xtensa Dual-Core LX6 @ 240 MHz
Validation:       ✅ Flash: 71.1% (931.6 KB) | RAM: 14.5% (47.6 KB)
Features:         Wi-Fi HTTP Dashboard (:80) + LittleFS Flash Storage + Web Serial CLI
```

#### Hardware Pin Connections
| Peripheral | Pin Function | ESP32 GPIO | Wiring Notes |
| :--- | :--- | :--- | :--- |
| **SSD1306 OLED** | SCL | `GPIO 22` | I²C Clock (3.3V, 4.7kΩ pull-up) |
| **SSD1306 OLED** | SDA | `GPIO 21` | I²C Data (3.3V, 4.7kΩ pull-up) |
| **PN532 RFID** | SCK | `GPIO 18` | SPI Clock |
| **PN532 RFID** | MISO | `GPIO 19` | SPI Master In Slave Out |
| **PN532 RFID** | MOSI | `GPIO 23` | SPI Master Out Slave In |
| **PN532 RFID** | SS / CS | `GPIO 5` | SPI Chip Select |
| **PN532 RFID** | RST | `GPIO 4` | Hardware Reset |
| **LED Green** | Anode (+) | `GPIO 26` | 220Ω series resistor to GND |
| **LED Red** | Anode (+) | `GPIO 27` | 220Ω series resistor to GND |
| **Buzzer** | Signal (+) | `GPIO 25` | Passive or Active buzzer |
| **Power Rails** | VCC / GND | `3V3` / `GND` | Power all peripherals strictly at 3.3V |

#### How to Prepare & Flash:
1. Connect DevKit to PC using a data-capable Micro-USB cable.
2. Confirm the COM port appears in Device Manager under `Ports (COM & LPT)`.
3. Execute the flash command:
   ```bash
   pio run -e esp32dev -t upload
   ```
4. **Bootloader Tip:** If the terminal displays `Connecting........_____.....`, press and hold the **`BOOT`** button on the board until writing begins, then release.

---

### 4.2 ESP32-S3 (DevKitC-1 / XIAO S3)

```
Target Flag:      pio run -e esp32_s3 -t upload
Architecture:     Xtensa Dual-Core LX7 @ 240 MHz (Vector Extensions)
Validation:       ✅ Flash: 26.9% (898.0 KB) | RAM: 14.5% (47.3 KB)
Features:         Native On-Chip USB CDC (Direct Browser Web Serial) + High Speed
```

#### Hardware Pin Connections
| Peripheral | Pin Function | ESP32-S3 GPIO | Wiring Notes |
| :--- | :--- | :--- | :--- |
| **SSD1306 OLED** | SCL / SDA | `GPIO 9` / `GPIO 8` | I²C Bus |
| **PN532 RFID** | SCK / MISO | `GPIO 12` / `GPIO 13` | SPI Bus (FSPI) |
| **PN532 RFID** | MOSI / CS | `GPIO 11` / `GPIO 10` | SPI Bus |
| **PN532 RFID** | RST | `GPIO 14` | Reset line |
| **Status LEDs** | Green / Red | `GPIO 1` / `GPIO 2` | 330Ω series resistors |
| **Buzzer** | Tone PWM | `GPIO 42` | Native LEDC PWM |

#### How to Prepare & Flash:
1. Connect via the USB-C port labelled **USB** (native CDC, GPIO 19/20) for optimal Web Serial performance.
2. Run upload:
   ```bash
   pio run -e esp32_s3 -t upload
   ```
3. **ROM Download Mode:** If upload does not start automatically, hold **`BOOT`** (GPIO 0), tap **`RST`**, release **`BOOT`**, and re-run.

---

### 4.3 ESP32-C3 (DevKitM-1 / XIAO C3 RISC-V)

```
Target Flag:      pio run -e esp32_c3 -t upload
Architecture:     32-bit RISC-V Single-Core @ 160 MHz
Validation:       ✅ Flash: 71.0% (931.0 KB) | RAM: 12.7% (41.4 KB)
Features:         Open RISC-V Standard + Integrated USB Serial/JTAG + Low Power
```

#### Hardware Pin Connections
| Peripheral | Pin Function | ESP32-C3 GPIO | Wiring Notes |
| :--- | :--- | :--- | :--- |
| **SSD1306 OLED** | SCL / SDA | `GPIO 9` / `GPIO 8` | Hardware I²C |
| **PN532 RFID** | SCK / MISO | `GPIO 4` / `GPIO 5` | Hardware SPI |
| **PN532 RFID** | MOSI / CS | `GPIO 6` / `GPIO 7` | Hardware SPI |
| **PN532 RFID** | RST | `GPIO 3` | Reset line |
| **Status LEDs** | Green / Red | `GPIO 0` / `GPIO 1` | 330Ω resistors |
| **Buzzer** | Tone PWM | `GPIO 2` | Hardware timer tone output |

#### How to Prepare & Flash:
1. Connect USB-C cable to PC.
2. Enter download mode if needed: hold **`BOOT`** (`GPIO 9`), press **`RST`**, release **`BOOT`**.
3. Upload firmware:
   ```bash
   pio run -e esp32_c3 -t upload
   ```

---

## 5. 🟪 STM32 Family Modular Guides

All STM32 boards run in **Dedicated High-Speed USB Serial Mode (115200 8N1)** with **MicroSD SPI persistent storage** and automatic **Volatile In-RAM Fallback**.

---

### 5.1 STM32F401 / F411 (WeAct Black Pill V3.0)

```
Target Flag:      pio run -e stm32_blackpill -t upload
Architecture:     ARM Cortex-M4 with FPU @ 84 MHz / 100 MHz
Validation:       ✅ Flash: 17.9% (93.8 KB) | RAM: 7.8% (7.6 KB)
Features:         Pocket form factor, direct USB-C DFU and CDC
```

#### Hardware Pin Connections
| Peripheral | Pin Function | Black Pill Pin | Physical Location / Notes |
| :--- | :--- | :--- | :--- |
| **SSD1306 OLED** | SCL / SDA | `PB6` / `PB7` | Left header (Hardware I2C1) |
| **PN532 RFID** | SCK / MISO / MOSI | `PA5` / `PA6` / `PA7` | Right header (Hardware SPI1) |
| **PN532 RFID** | CS / RST | `PB0` / `PB1` | Right header |
| **MicroSD SPI** | CS / SCK / MISO / MOSI | `PA4` / `PA5` / `PA6` / `PA7` | Shared SPI1 bus with dedicated `PA4` CS |
| **Status LEDs** | Green / Red | `PA1` / `PA2` | 330Ω resistors |
| **Buzzer** | Tone Output | `PB8` | Hardware timer tone generation |
| **Power Rails** | 3.3V / GND | `3V3` / `GND` | 3.3V power bus |

#### How to Prepare & Flash:

##### Method A: Direct USB-C ROM DFU (No extra hardware needed!)
1. Connect Black Pill to PC via USB Type-C.
2. Hold down the **`BOOT0`** button.
3. Tap and release the **`NRST`** button once.
4. Release the **`BOOT0`** button.
5. In Windows Device Manager, verify that `STM32 BOOTLOADER` appears. (If driver is missing, open [Zadig](https://zadig.akeo.ie/) and install `WinUSB` for `STM32 BOOTLOADER`).
6. Flash via PlatformIO:
   ```bash
   pio run -e stm32_blackpill -t upload
   ```

##### Method B: ST-LINK v2 Dongle (SWD 4-Wire)
1. Wire ST-Link v2 to the SWD header: `3V3` ➔ `3.3V`, `GND` ➔ `GND`, `SWDIO` ➔ `DIO`, `SWCLK` ➔ `CLK`.
2. Run `pio run -e stm32_blackpill -t upload`.

---

### 5.2 STM32F446RE (ST Nucleo-64)

```
Target Flag:      pio run -e stm32_nucleo_f446re -t upload
Architecture:     ARM Cortex-M4 with FPU @ 180 MHz (High Performance)
Validation:       ✅ Flash: 16.6% (86.9 KB) | RAM: 3.2% (4.2 KB)
Features:         Integrated ST-LINK v2.1 with Hardware Virtual COM Port on USART2
```

#### Hardware Pin Connections (Morpho & Arduino Headers)
| Peripheral | Pin Function | Nucleo Morpho Pin | Arduino Header Equivalent |
| :--- | :--- | :--- | :--- |
| **SSD1306 OLED** | SCL / SDA | `PB6` / `PB7` | `D10` / `D4` (or `D15`/`D14` I2C1) |
| **PN532 RFID** | SCK / MISO / MOSI | `PA5` / `PA6` / `PA7` | `D13` / `D12` / `D11` (SPI1) |
| **PN532 RFID** | CS / RST | `PB0` / `PB1` | `A3` / `A4` |
| **MicroSD SPI** | CS | `PA4` | `A2` |
| **Status LEDs** | Green / Red | `PA1` / `PA2` | Morpho Left Header |
| **Buzzer** | Tone Output | `PB8` | `D15` |
| **Power Rails** | 3.3V / GND | `3V3` / `GND` | Power header |

#### How to Prepare & Flash:
1. Connect standard USB cable to CN1 (the ST-LINK USB connector at the top of the board).
2. The board enumerates immediately as `ST-Link Debugger`, `Virtual COM Port`, and a mass storage volume.
3. Flash directly with zero button presses:
   ```bash
   pio run -e stm32_nucleo_f446re -t upload
   ```
4. The board programs in ~3 seconds and restarts into Attendly mode.

---

### 5.3 STM32G474RE (ST Nucleo-64 Modern)

```
Target Flag:      pio run -e stm32_nucleo_g474re -t upload
Architecture:     ARM Cortex-M4 with FPU & Math Accelerators @ 170 MHz
Validation:       ✅ Flash: 17.8% (93.2 KB) | RAM: 3.4% (4.4 KB)
Features:         Next-Gen ST-LINK v3 + Ultra-low jitter hardware timers
```

#### Hardware Pin Connections:
* **OLED:** `PB6` (SCL), `PB7` (SDA)
* **PN532 RFID:** `PA5` (SCK), `PA6` (MISO), `PA7` (MOSI), `PB0` (CS), `PB1` (RST)
* **MicroSD:** `PA4` (CS)
* **Feedback:** `PA1` (Green LED), `PA2` (Red LED), `PB8` (Buzzer)

#### How to Flash:
```bash
pio run -e stm32_nucleo_g474re -t upload
```

---

### 5.4 STM32G491RE (ST Nucleo-64 Access Line)

```
Target Flag:      pio run -e stm32_nucleo_g491re -t upload
Architecture:     ARM Cortex-M4 with FPU @ 170 MHz
Validation:       ✅ Flash: 17.7% (92.7 KB) | RAM: 3.8% (4.4 KB)
Features:         ST-LINK/V3E + Low power run mode + High analog integration
```

> [!NOTE]
> **Toolchain Note:** The upstream PlatformIO `ststm32` platform definition for `nucleo_g491re` omits Arduino framework support by default. A pre-configured project board definition is included in `hardware/firmware/boards/nucleo_g491re.json` which maps the official STM32 Arduino core variant `STM32G4xx/G491RC(I-T)_G491RE(I-T-Y)x(Z)_G4A1RE(I-T-Y)` with zero code modifications needed.

#### Hardware Pin Connections:
* **OLED:** `PB6` (SCL), `PB7` (SDA)
* **PN532 RFID:** `PA5` (SCK), `PA6` (MISO), `PA7` (MOSI), `PB0` (CS), `PB1` (RST)
* **MicroSD:** `PA4` (CS)
* **Feedback:** `PA1` (Green LED), `PA2` (Red LED), `PB8` (Buzzer)

#### How to Flash:
```bash
pio run -e stm32_nucleo_g491re -t upload
```

---

### 5.5 STM32L476RG (ST Nucleo-64 Ultra-Low Power)

```
Target Flag:      pio run -e stm32_nucleo_l476rg -t upload
Architecture:     ARM Cortex-M4 with FPU @ 80 MHz (Ultra-Low Power)
Validation:       ✅ Flash: 8.7% (91.4 KB) | RAM: 4.5% (4.4 KB)
Features:         1 MB Flash + Sub-microamp sleep states (ideal for battery/solar kiosks)
```

#### How to Flash:
```bash
pio run -e stm32_nucleo_l476rg -t upload
```

---

### 5.6 STM32F746ZG (ST Nucleo-144 High-End)

```
Target Flag:      pio run -e stm32_nucleo_f746zg -t upload
Architecture:     ARM Cortex-M7 with Double-Precision FPU & L1 Cache @ 216 MHz
Validation:       ✅ Flash: 8.8% (91.5 KB) | RAM: 1.3% (4.4 KB)
Features:         High-capacity development board, capable of storing thousands of users in RAM
```

#### Hardware Pin Connections (ST Zio & Morpho):
* **OLED:** `PB8` (SCL), `PB9` (SDA)
* **PN532 RFID:** `PA5` (SCK), `PA6` (MISO), `PA7` (MOSI), `PD14` (CS), `PF12` (RST)
* **MicroSD:** `PD15` (CS)
* **Buzzer:** `PB0`

#### How to Flash:
```bash
pio run -e stm32_nucleo_f746zg -t upload
```

---

### 5.7 STM32F103C8 / CB (Blue Pill 128K)

```
Target Flag:      pio run -e stm32_bluepill -t upload
Architecture:     ARM Cortex-M3 @ 72 MHz
Validation:       ✅ Flash: 64.0% (83.9 KB / 128 KB) | RAM: 20.2% (4.1 KB / 20 KB)
Features:         Ubiquitous low-cost standard, supported via 128KB flash allocation
```

> [!IMPORTANT]
> **Flash Boundary Note:**  
> The embedded firmware stack (USB CDC + U8g2 graphics + ArduinoJson v7 parser + SD FAT filesystem) compiles to **83.9 KB**.  
> If configured for standard 64 KB, the linker will report `region FLASH overflowed by 18744 bytes`.  
> Virtually all STM32F103C8 chips on the market actually possess 128 KB of working Flash silicon on the die. In `platformio.ini`, `stm32_bluepill` is configured with `board = bluepill_f103c8_128k` to enable this capability seamlessly.

#### Hardware Pin Connections:
* **OLED:** `PB6` (SCL), `PB7` (SDA)
* **PN532 RFID:** `PA5` (SCK), `PA6` (MISO), `PA7` (MOSI), `PB0` (CS), `PB1` (RST)
* **MicroSD:** `PA4` (CS)
* **Indicators:** `PA1` (Green LED), `PA2` (Red LED), `PB8` (Buzzer)

#### How to Flash:
* **Method A (ST-Link v2):** Connect `SWDIO`, `SWCLK`, `GND`, `3V3` and run:
  ```bash
  pio run -e stm32_bluepill -t upload
  ```
* **Method B (USB-to-UART CP2102/CH340):** Wire `TX` ➔ `PA10`, `RX` ➔ `PA9`, set **`BOOT0` jumper to 1**, press **`RESET`**, and run:
  ```bash
  pio run -e stm32_bluepill -t upload --upload-port COMx
  ```
  *(Move `BOOT0` jumper back to 0 and reset after flashing).*

---

## 6. 🔌 Web Serial CLI & PWA Integration

The Attendly PWA connects directly to the terminal using the **W3C Web Serial API** in Chromium browsers (Chrome, Edge, Opera, Brave).

> 🌐 **Live Hosted Testing PWA (Directly from Repo):**  
> For bench testing and interactive experimentation without setting up a local web server, use the live deployment:  
> **👉 [https://serial-attendance-pwa.vercel.app/](https://serial-attendance-pwa.vercel.app/)**

### Connection Parameters:
* **Baud Rate:** `115200`
* **Data Bits:** `8`
* **Stop Bits:** `1`
* **Parity:** `None`
* **Line Termination:** `\n` (Newline)

### Built-in Serial CLI Commands:
You can test the terminal via the Attendly Web Serial console or any terminal emulator:

| Command | Arguments | Description / Example |
| :--- | :--- | :--- |
| `help` | — | Prints all available commands and firmware build metadata |
| `status` | — | Displays terminal uptime, state machine phase, and free heap |
| `scan` | `<HEX_UID>` | Simulates an RFID badge tap (e.g. `scan A1B2C3D4`) |
| `users` | — | Lists all enrolled users and their card UIDs |
| `add` | `<UID> <Name>`| Enrolls a student (e.g. `add A1B2C3D4 Alice_Smith`) |
| `del` | `<UID>` | Removes an enrolled student from storage |
| `dump` | — | Outputs complete attendance session log in JSON format |
| `diag` | — | Runs self-diagnostics on OLED, RFID, and SD storage |
| `wipe` | — | Clears all registered cards and logs |
| `reboot`| — | Safely synchronizes filesystem and resets MCU |

---

## 7. 🏗️ Software Architecture & Dependency Injection

The firmware is structured into modular layers adhering to clean SOLID architecture principles:

```
┌────────────────────────────────────────────────────────┐
│               Application Layer (app.cpp)              │
│       • State Machine (INIT -> READY -> MARKED)        │
│       • CLI Command Parser & Serial Protocol           │
└──────────────────────────┬─────────────────────────────┘
                           │ Consumes Pure Abstract Interfaces
┌──────────────────────────▼─────────────────────────────┐
│              Hardware Abstraction Interfaces           │
│    • IDisplay    • IRfid    • IBuzzer    • ILed        │
│    • IAttendanceRepository  • IPlatform                │
└────────────┬─────────────────────────────┬─────────────┘
             │ Implements                  │ Implements
┌────────────▼──────────────┐ ┌────────────▼─────────────┐
│    ESP32 Drivers Layer    │ │    STM32 Drivers Layer   │
│ • SSD1306 Display Driver  │ │ • STM32 I2C Display      │
│ • MFRC522 / PN532 SPI     │ │ • STM32 SPI RFID         │
│ • ESP32 LEDC PWM Buzzer   │ │ • Hardware Timer Buzzer  │
│ • LittleFS Flash Repo     │ │ • MicroSD / In-RAM Repo  │
└───────────────────────────┘ └──────────────────────────┘
```

---

## 8. ❓ Troubleshooting & FAQ

#### Q: PlatformIO reports `Access is denied` or `Could not open port COMx`
* **Cause:** The Attendly PWA or a serial monitor is actively connected to the COM port.
* **Fix:** Click **Disconnect** in Attendly, run `pio run -e <target> -t upload`, and click **Connect** again once flashing finishes.

#### Q: The OLED display stays blank on boot
* **Cause:** I²C address mismatch or missing pull-up resistors.
* **Fix:** Verify `SDA`/`SCL` lines are wired correctly. The firmware checks both `0x3C` and `0x3D` addresses automatically.

#### Q: The OLED displays `RFID: SIM MODE` on startup
* **Cause:** The PN532/RC522 reader did not acknowledge SPI initialization.
* **Fix:** Check SPI wiring and CS pin. The firmware gracefully falls back to Simulation Mode so you can still test by sending `scan <UID>` over the Web Serial console.

#### Q: Does STM32 require an external MicroSD card module?
* **Answer:** No. While a MicroSD card provides persistent non-volatile storage, the firmware features an automatic **In-RAM Volatile Fallback**. If no SD card is detected, all registration and attendance operations function directly in high-speed RAM.

---

## 📄 License & Project Intent

Created as an **open-source educational & base reference project** for embedded IoT attendance systems.  
Licensed under the **MIT License** — free to study, modify, and expand for personal, academic, or custom hardware applications.
