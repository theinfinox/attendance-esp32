// ============================================================================
// utils.cpp — Utility Functions Implementation
// ============================================================================

#include "app_utils.h"

#if defined(ARDUINO_ARCH_STM32)
extern "C" char* sbrk(int incr);
#endif

namespace Utils {

String uidToHexString(const byte* uid, byte length) {
    String hex;
    hex.reserve(length * 2);
    for (byte i = 0; i < length; i++) {
        if (uid[i] < 0x10) {
            hex += '0';
        }
        hex += String(uid[i], HEX);
    }
    hex.toUpperCase();
    return hex;
}

String formatMacAddress(const uint8_t* mac) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

uint32_t freeHeap() {
#if defined(ESP32)
    return ESP.getFreeHeap();
#elif defined(ARDUINO_ARCH_STM32)
    char top;
    return &top - reinterpret_cast<char*>(sbrk(0));
#else
    return 0;
#endif
}

String uptimeString() {
    unsigned long totalSec = millis() / 1000;
    unsigned int hours   = totalSec / 3600;
    unsigned int minutes = (totalSec % 3600) / 60;
    unsigned int seconds = totalSec % 60;

    char buf[12];
    snprintf(buf, sizeof(buf), "%02u:%02u:%02u", hours, minutes, seconds);
    return String(buf);
}

} // namespace Utils
