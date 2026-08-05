#pragma once
// ============================================================================
// pins.h — Centralized GPIO Pin Definitions
// All hardware pin assignments in one place. Never hardcode pins elsewhere.
// ============================================================================

// --- RFID Reader (MFRC522 — SPI) ---
constexpr int PIN_RFID_SDA  = 5;   // SPI Slave Select
constexpr int PIN_RFID_SCK  = 18;  // SPI Clock
constexpr int PIN_RFID_MOSI = 23;  // SPI Master Out
constexpr int PIN_RFID_MISO = 19;  // SPI Master In
constexpr int PIN_RFID_RST  = 4;   // Reset

// --- OLED Display (SSD1306 — I²C) ---
constexpr int PIN_OLED_SDA  = 21;
constexpr int PIN_OLED_SCL  = 22;

// --- Indicators ---
constexpr int PIN_LED_GREEN = 26;
constexpr int PIN_LED_RED   = 27;
constexpr int PIN_BUZZER    = 25;
