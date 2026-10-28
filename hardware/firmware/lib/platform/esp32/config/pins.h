#pragma once
// ============================================================================
// pins.h — Centralized ESP32 GPIO Pin Definitions
// All hardware pin assignments in one place. Never hardcode pins elsewhere.
// ============================================================================

#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(ARDUINO_ESP32C3_DEV)
// --- ESP32-C3 Pinout (Compact 22-pin RISC-V) ---
constexpr int PIN_RFID_SDA  = 7;   // SPI Slave Select
constexpr int PIN_RFID_SCK  = 4;   // SPI Clock
constexpr int PIN_RFID_MOSI = 6;   // SPI Master Out
constexpr int PIN_RFID_MISO = 5;   // SPI Master In
constexpr int PIN_RFID_RST  = 3;   // Reset

constexpr int PIN_OLED_SDA  = 8;   // I2C SDA
constexpr int PIN_OLED_SCL  = 9;   // I2C SCL

constexpr int PIN_LED_GREEN = 0;
constexpr int PIN_LED_RED   = 1;
constexpr int PIN_BUZZER    = 2;

#elif defined(CONFIG_IDF_TARGET_ESP32S3)
// --- ESP32-S3 Pinout (Xtensa Dual-Core) ---
constexpr int PIN_RFID_SDA  = 10;  // SPI Slave Select
constexpr int PIN_RFID_SCK  = 12;  // SPI Clock
constexpr int PIN_RFID_MOSI = 11;  // SPI Master Out
constexpr int PIN_RFID_MISO = 13;  // SPI Master In
constexpr int PIN_RFID_RST  = 14;  // Reset

constexpr int PIN_OLED_SDA  = 8;   // I2C SDA
constexpr int PIN_OLED_SCL  = 9;   // I2C SCL

constexpr int PIN_LED_GREEN = 1;
constexpr int PIN_LED_RED   = 2;
constexpr int PIN_BUZZER    = 42;

#else
// --- Standard ESP32 (WROVER / WROOM / DevKit) Pinout ---
constexpr int PIN_RFID_SDA  = 5;   // SPI Slave Select
constexpr int PIN_RFID_SCK  = 18;  // SPI Clock
constexpr int PIN_RFID_MOSI = 23;  // SPI Master Out
constexpr int PIN_RFID_MISO = 19;  // SPI Master In
constexpr int PIN_RFID_RST  = 4;   // Reset

constexpr int PIN_OLED_SDA  = 21;  // I2C SDA
constexpr int PIN_OLED_SCL  = 22;  // I2C SCL

constexpr int PIN_LED_GREEN = 26;
constexpr int PIN_LED_RED   = 27;
constexpr int PIN_BUZZER    = 25;
#endif
