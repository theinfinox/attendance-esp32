# Standalone STM32G491RE RFID Attendance Terminal (Flash-Only Architecture)

A simplified, self-contained RFID attendance firmware for the STMicroelectronics **NUCLEO-G491RE** (STM32G491RET6). Designed specifically as a streamlined college project, this firmware stores all registered students and attendance records persistently in the MCU's **internal Flash memory**, eliminating MicroSD card hardware requirements and third-party filesystem dependencies while maintaining 100% backward compatibility with the existing Serial Progressive Web App (PWA) frontend.

---

## 1. Hardware Pinout & Wiring

| Peripheral | Signal | STM32 Pin | Arduino Header | Description |
|---|---|---|---|---|
| **PN532 RFID** | SCK | `PA5` | `D13` (CN5 pin 6) | SPI1 Clock (Alternate Function AF5) |
| **PN532 RFID** | MISO | `PA6` | `D12` (CN5 pin 5) | SPI1 MISO |
| **PN532 RFID** | MOSI | `PA7` | `D11` (CN5 pin 4) | SPI1 MOSI (Transmits LSB first) |
| **PN532 RFID** | NSS / CS | `PB6` | `D10` (CN5 pin 3) | SPI1 Slave Select |
| **PN532 RFID** | VCC | `5V` or `3V3` | `5V` (CN6 pin 5) | Power supply rail |
| **PN532 RFID** | GND | `GND` | `GND` (CN6 pin 7) | Ground rail |
| **OLED (SSD1306)** | SCL | `PA9` | `D8` (CN5 pin 1) | I2C SCL |
| **OLED (SSD1306)** | SDA | `PA8` | `D7` (CN9 pin 8) | I2C SDA |

*Note: MicroSD card is not required. `PA4` is left unconnected.*

---

## 2. PN532 DIP Switch Configuration (SW1)

To select **Hardware SPI mode**, configure the 2-position SMD DIP switch `SW1`:

| Switch Label | Internal Line | Setting | Position |
|---|---|---|---|
| **1** | $I_0$ | **OFF (0)** | Away from the "ON" label |
| **2** | $I_1$ | **ON (1)** | Towards the "ON" label |

> [!IMPORTANT]
> The PN532 only samples the DIP switch states on cold power-on. If you change switch positions while powered, **disconnect USB power for at least 5 seconds** to allow onboard capacitors to discharge completely.

---

## 3. Dedicated On-Chip Flash Storage

* **Total Flash**: 512 KB (`0x08000000` to `0x0807FFFF`).
* **Application Firmware**: `0x08000000` to `0x0805FFFF` (384 KB maximum, enforced by linker flag `-Wl,--defsym=LD_MAX_SIZE=0x70000`).
* **Dedicated Storage Partition**: `0x08070000` to `0x0807FFFF` (64 KB / 32 pages of 2 KB each):
  * **Student Database (16 KB / Pages 224..231)**:
    * Uses **Ping-Pong A/B Dual-Bank** atomic commits (8 KB per bank).
    * Up to **126 enrolled students**.
    * Updates are written to the inactive bank with an incremented sequence number and validated with CRC32 before switching active pointers.
    * 100% immune to data corruption if power is interrupted during writes.
  * **Attendance Event Journal (48 KB / Pages 232..255)**:
    * Append-only sequential storage of **48-byte packed records** with CRC16.
    * Retention Capacity: **1,024 attendance punches**.
    * Zero page erasures during normal attendance logging.
    * **Exhaustion Guard**: When 1,024 records are reached, the system **never overwrites** old punches; it halts new writes, displays `"Storage Full"`, and warns over serial until `CLEAR_LOGS` is executed.

---

## 4. Build, Upload & Serial Commands

### Compile Firmware
```bash
pio run -d standalone_g491re
```

### Flash to NUCLEO-G491RE (via ST-Link)
```bash
pio run -d standalone_g491re -t upload
```

### PWA Web Serial Commands (115200 baud)
* `STATUS`: Returns `{"type":"status","data":{"board":"STM32","firmware":"1.0.0","storage":"FLASH","rfid":"...","users":N,"logs":N}}` followed by `{"type":"done"}`.
* `ADD_USER {"uid":"04A1B2C3","name":"Alice","role":"Student"}`: Atomically registers a student in on-chip Flash.
* `DELETE_USER <uid>`: Atomically deletes a student from on-chip Flash.
* `DUMP_USERS`: Streams `{"<uid>":"<name>",...}` matching PWA user table.
* `DUMP_LOGS`: Streams all stored punches sequentially.
* `CLEAR_LOGS`: Erases the 24 journal pages and resets punch counter to 0.
