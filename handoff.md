# ESP32 Smart Attendance System - Project Handoff

## 1. Project Overview
A robust, offline-capable Smart Attendance System built on the ESP32 (WROVER-E) platform. The system uses an MFRC522 (PN532) RFID reader for scanning ID cards and an SSD1306 OLED display for real-time user feedback.

Currently, the project is operating in **Standalone Mode** (`CFG_STANDALONE_MODE = true`), meaning it functions completely independently without needing a cloud backend. It hosts its own Web Dashboard on port 80 and uses the ESP32's internal flash memory (LittleFS) as an embedded database.

## 2. Current State & Architecture
The codebase is structured using a heavily decoupled, object-oriented design to ensure stability and ease of porting to other platforms.

### Core Architecture
*   **State Machine (`StateMachine` / `App`)**: The central brain of the system. It handles all logical transitions (e.g., `READY` → `CARD_DETECTED` → `SENDING` → `SUCCESS`) and ensures the system never gets stuck in a blocking loop.
*   **Decoupled Managers**: Hardware peripherals are wrapped in single-responsibility classes (`DisplayManager`, `RfidManager`, `LedManager`, `BuzzerManager`, `WiFiManager`, `WebManager`, `Storage`). The `App` orchestrates these managers without needing to know their internal hardware logic.

### Features Implemented
*   **Non-Blocking RFID Reading**: Uses the PN532 over software SPI, implementing debouncing to prevent multiple reads of the same card in a single tap.
*   **Dynamic OLED Display**: Driven by the `U8g2` library over I2C, it shows real-time statuses (Booting, WiFi Connecting, IP Address, Card Detected, Success, and various error states).
*   **Embedded Web Dashboard**: A modern, mobile-responsive HTML/CSS/JS frontend served directly from the ESP32 via `<WebServer.h>`.
*   **LittleFS Persistent Storage**: 
    *   `users.json`: Stores registered Card UIDs and associated Employee Names.
    *   `logs.json`: Stores attendance timestamps. Automatically purges older logs to prevent flash wear (keeps the last 50 entries).
*   **Web API Endpoints**:
    *   `GET /api/users`: Fetch registered users.
    *   `POST /api/users`: Register a new card.
    *   `DELETE /api/users`: Delete a registered user.
    *   `GET /api/logs`: Fetch recent attendance logs.
    *   `DELETE /api/logs`: Clear all attendance logs.
    *   `GET /api/last_scan`: Polls the RFID reader for the most recently scanned UID to populate the registration form.
*   **UI Config Toggles**: `CFG_SHOW_UID_ON_DETECT` in `config.h` allows the admin to decide if the raw UID should be displayed during the "Processing..." screen for privacy.

## 3. Hardware Wiring Guide

All pins are centralized in `include/pins.h`. If you need to change a pin, change it there—do not hardcode pins in the manager classes.

### MFRC522 / PN532 RFID Reader (Software SPI)
*   **SCK**  -> GPIO 18
*   **MISO** -> GPIO 19
*   **MOSI** -> GPIO 23
*   **SS (SDA)** -> GPIO 5
*   **RST**  -> GPIO 4
*   **VCC**  -> 3.3V
*   **GND**  -> GND

### SSD1306 OLED Display (I2C)
*   **SDA**  -> GPIO 21
*   **SCL**  -> GPIO 22
*   **VCC**  -> 3.3V
*   **GND**  -> GND

### External Indicators (Not yet physically wired, but defined)
*   **Green LED** -> GPIO 26
*   **Red LED**   -> GPIO 27
*   **Buzzer**    -> GPIO 25

## 4. Next Steps & Known Limitations
1.  **Time Sync (NTP)**: Currently, the timestamp relies on the basic `time()` function which defaults to Unix Epoch 0 unless synced via NTP. In a real-world scenario, the `WiFiManager` needs to reach out to an NTP server upon connection to set the internal RTC.
2.  **Cloud Integration**: Once ready, `CFG_STANDALONE_MODE` in `config.h` can be set to `false`. You will then need to finish implementing the `ApiClient` using `HTTPClient` to send payloads to `CFG_SERVER_URL`.
3.  **Security**: The Web Dashboard currently has no authentication. Anyone on the Wi-Fi network who knows the IP address can add/delete cards and clear logs. A simple Basic Auth middleware should be added to the `WebServer` routes.
