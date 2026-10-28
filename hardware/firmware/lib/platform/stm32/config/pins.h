#pragma once
// ============================================================================
// pins.h — STM32 Centralized Pin Definitions (e.g. Black Pill / Nucleo / Blue Pill)
// ============================================================================

// --- RFID Reader (PN532 / MFRC522 — SPI) ---
#ifndef PIN_RFID_SCK
constexpr int PIN_RFID_SCK  = PA5;  // SPI1 Clock
#endif
#ifndef PIN_RFID_MISO
constexpr int PIN_RFID_MISO = PA6;  // SPI1 MISO
#endif
#ifndef PIN_RFID_MOSI
constexpr int PIN_RFID_MOSI = PA7;  // SPI1 MOSI
#endif
#ifndef PIN_RFID_SDA
constexpr int PIN_RFID_SDA  = PB0;  // SPI1 Slave Select (SS / CS)
#endif
#ifndef PIN_RFID_RST
constexpr int PIN_RFID_RST  = PB1;  // Reset
#endif

// --- MicroSD Card Module (SPI CS) ---
#ifndef PIN_SD_CS
constexpr int PIN_SD_CS     = PA4;  // Chip Select for SD card SPI
#endif

// --- OLED Display (SSD1306 — I2C1) ---
#ifndef PIN_OLED_SCL
constexpr int PIN_OLED_SCL  = PB6;  // I2C1 SCL
#endif
#ifndef PIN_OLED_SDA
constexpr int PIN_OLED_SDA  = PB7;  // I2C1 SDA
#endif

// --- Indicators ---
#ifndef PIN_LED_GREEN
constexpr int PIN_LED_GREEN = PA1;
#endif
#ifndef PIN_LED_RED
constexpr int PIN_LED_RED   = PA2;
#endif
#ifndef PIN_BUZZER
constexpr int PIN_BUZZER    = PB8;
#endif
