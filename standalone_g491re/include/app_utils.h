#pragma once
// ============================================================================
// utils.h — Utility Functions
// ============================================================================

#include <Arduino.h>

namespace Utils {

// Convert raw UID bytes to uppercase hex string (e.g., "A1B2C3D4")
String uidToHexString(const byte* uid, byte length);

// Format MAC address as "AA:BB:CC:DD:EE:FF"
String formatMacAddress(const uint8_t* mac);

// Get free heap memory in bytes
uint32_t freeHeap();

// Get uptime string "HH:MM:SS"
String uptimeString();

} // namespace Utils
