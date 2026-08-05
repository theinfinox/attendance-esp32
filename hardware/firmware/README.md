# IoT RFID Attendance Terminal — Firmware

ESP32-based RFID attendance terminal with OLED display, LED indicators, and buzzer feedback. Reads RFID cards, communicates with a backend REST API (currently mocked), and provides real-time visual/audio user feedback.

## Hardware

| Component | Model | Interface |
|-----------|-------|-----------|
| MCU | ESP32-WROVER-E | — |
| RFID Reader | HW-147 (MFRC522) | SPI |
| OLED Display | SSD1306 128×64 | I²C |
| Green LED | 5mm | GPIO |
| Red LED | 5mm | GPIO |
| Buzzer | Passive | GPIO (PWM) |

## Wiring Diagram

### RFID Reader (MFRC522 → ESP32)

| MFRC522 Pin | ESP32 GPIO | Wire Color (suggested) |
|-------------|------------|----------------------|
| SDA/SS | GPIO 5 | Yellow |
| SCK | GPIO 18 | Orange |
| MOSI | GPIO 23 | Green |
| MISO | GPIO 19 | Blue |
| RST | GPIO 4 | White |
| 3.3V | 3V3 | Red |
| GND | GND | Black |

### OLED Display (SSD1306 → ESP32)

| SSD1306 Pin | ESP32 GPIO |
|-------------|------------|
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| VCC | 3V3 |
| GND | GND |

### Indicators

| Component | ESP32 GPIO | Notes |
|-----------|------------|-------|
| Green LED (+) | GPIO 26 | 220Ω resistor to GND |
| Red LED (+) | GPIO 27 | 220Ω resistor to GND |
| Buzzer (+) | GPIO 25 | Passive buzzer |

```
                    ┌──────────────────┐
                    │   ESP32-WROVER-E │
                    │                  │
    MFRC522 SDA ────┤ GPIO 5      3V3 ├──── MFRC522 3.3V / OLED VCC
    MFRC522 RST ────┤ GPIO 4      GND ├──── Common GND
    MFRC522 SCK ────┤ GPIO 18         │
    MFRC522 MISO ───┤ GPIO 19         │
    MFRC522 MOSI ───┤ GPIO 23         │
                    │                  │
      OLED SDA ─────┤ GPIO 21         │
      OLED SCL ─────┤ GPIO 22         │
                    │                  │
     Buzzer (+) ────┤ GPIO 25         │
   Green LED (+) ───┤ GPIO 26         │
     Red LED (+) ───┤ GPIO 27         │
                    └──────────────────┘
```

## Project Structure

```
firmware/
├── platformio.ini          # PlatformIO configuration
├── README.md               # This file
├── include/
│   ├── pins.h              # Centralized GPIO definitions
│   ├── config.h            # WiFi, server URL, device ID
│   ├── constants.h         # Timing, retry, threshold values
│   ├── version.h           # Firmware version
│   ├── types.h             # Shared enums and structs
│   └── logger.h            # Serial logging macros
└── src/
    ├── main.cpp            # Entry point (setup/loop → App)
    ├── app.h/cpp           # Application orchestrator
    ├── state_machine.h/cpp # Firmware state machine
    ├── wifi_manager.h/cpp  # WiFi connect/reconnect
    ├── rfid_manager.h/cpp  # MFRC522 card reading
    ├── display_manager.h/cpp # SSD1306 OLED rendering
    ├── led_manager.h/cpp   # Non-blocking LED patterns
    ├── buzzer_manager.h/cpp # Non-blocking buzzer tones
    ├── api_client.h/cpp    # Backend API (mocked)
    ├── storage.h/cpp       # Offline storage (stub)
    └── utils.h/cpp         # Utility functions
```

## Architecture

```
┌─────────────┐
│  main.cpp   │  setup() / loop()
└──────┬──────┘
       │
┌──────▼──────┐
│     App     │  Orchestration
└──────┬──────┘
       │
┌──────▼──────┐
│ StateMachine│  State transitions
└──────┬──────┘
       │
┌──────▼───────────────────────────────┐
│            Managers                   │
│ WiFi │ RFID │ Display │ LED │ Buzzer │
│ API  │ Storage │ Utils               │
└──────┬───────────────────────────────┘
       │
┌──────▼──────┐
│ Hardware    │  pins.h (GPIO only)
└─────────────┘
```

## State Machine

```
BOOT → INIT → WIFI_CONNECTING → BACKEND_CONNECTING → READY
                                                        │
                                              ┌─────────▼─────────┐
                                              │   CARD_DETECTED   │
                                              └─────────┬─────────┘
                                                        │
                                              ┌─────────▼─────────┐
                                              │     SENDING       │
                                              └───┬───────────┬───┘
                                                  │           │
                                         ┌────────▼──┐  ┌─────▼───────┐
                                         │  SUCCESS  │  │   FAILED    │
                                         └────────┬──┘  └─────┬───────┘
                                                  │           │
                                                  └─────┬─────┘
                                                        │ (auto-return)
                                                  ┌─────▼─────┐
                                                  │   READY    │
                                                  └───────────┘

              NETWORK_ERROR ←── (any state on WiFi loss)
              NETWORK_ERROR ──→ WIFI_CONNECTING (auto-reconnect)
```

## Required Libraries

| Library | Version | Purpose |
|---------|---------|---------|
| MFRC522 | ^1.4.10 | RFID reader driver |
| U8g2 | ^2.35.7 | OLED display driver |
| ArduinoJson | ^7.0.4 | JSON parsing (for future API) |

## Build Instructions

### Prerequisites

1. Install [VS Code](https://code.visualstudio.com/)
2. Install [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)

### Configure

Edit `include/config.h` with your WiFi credentials:

```cpp
constexpr const char* CFG_WIFI_SSID     = "YOUR_WIFI_SSID";
constexpr const char* CFG_WIFI_PASSWORD  = "YOUR_WIFI_PASSWORD";
```

### Build

```bash
cd hardware/firmware
pio run
```

### Upload

```bash
pio run --target upload
```

### Monitor Serial Output

```bash
pio device monitor
```

## LED Feedback Patterns

| Event | LED | Pattern |
|-------|-----|---------|
| Success | Green | 1 short blink |
| Duplicate | Red | 2 short blinks |
| Failure | Red | 1 long blink |

## Buzzer Feedback Patterns

| Event | Frequency | Pattern |
|-------|-----------|---------|
| Success | 2000 Hz | 1 short beep |
| Duplicate | 2000 Hz | 2 short beeps |
| Unknown Card | 500 Hz | 1 long beep |
| Network Error | 500 Hz | 3 short beeps |

## Mock Mode

The firmware ships with a **mocked API client** that cycles through responses:

1. **SUCCESS** — "Attendance Marked!"
2. **DUPLICATE** — "Already Marked"
3. **UNKNOWN_CARD** — "Unknown Card"

This lets you demo the complete user flow without a backend. When the real backend is ready, swap the internals of `ApiClient` — no other module changes needed.

## Memory Usage

| Resource | Used | Available | Usage |
|----------|------|-----------|-------|
| RAM | ~45 KB | 320 KB | ~14% |
| Flash | ~792 KB | 1280 KB | ~61% |

## Future Enhancements

- [ ] Real HTTPS API integration
- [ ] AP/captive portal WiFi provisioning
- [ ] SPIFFS/NVS offline attendance storage
- [ ] OTA firmware updates
- [ ] STM32 port (architecture supports it)
- [ ] Student name display on success

## License

MIT
