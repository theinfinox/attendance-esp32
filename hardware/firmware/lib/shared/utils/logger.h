#pragma once
// ============================================================================
// logger.h — Serial Logging Macros
// Usage: LOG_INFO("WiFi connected");  LOG_ERROR("RFID init failed");
// ============================================================================

#include <Arduino.h>
#include "config.h"

// Log levels: 0=OFF, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG
#ifndef LOG_LEVEL
#define LOG_LEVEL (CFG_DEBUG_MODE ? 4 : 1)
#endif

#define LOG_ERROR(msg, ...) \
    do { if (LOG_LEVEL >= 1) { Serial.printf("[ERROR] " msg "\n", ##__VA_ARGS__); } } while(0)

#define LOG_WARN(msg, ...) \
    do { if (LOG_LEVEL >= 2) { Serial.printf("[WARN]  " msg "\n", ##__VA_ARGS__); } } while(0)

#define LOG_INFO(msg, ...) \
    do { if (LOG_LEVEL >= 3) { Serial.printf("[INFO]  " msg "\n", ##__VA_ARGS__); } } while(0)

#define LOG_DEBUG(msg, ...) \
    do { if (LOG_LEVEL >= 4) { Serial.printf("[DEBUG] " msg "\n", ##__VA_ARGS__); } } while(0)
