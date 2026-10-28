#if defined(ARDUINO_ARCH_STM32)
// ============================================================================
// rfid_manager.cpp — STM32 PN532 RFID Reader Manager (Software SPI)
// ============================================================================

#include "rfid_manager.h"
#include "app_utils.h"
#include "logger.h"
#include "pins.h"
#include "config.h"

STM32RfidManager::STM32RfidManager()
    : _pn532(PIN_RFID_SCK, PIN_RFID_MISO, PIN_RFID_MOSI, PIN_RFID_SDA)
{
}

void STM32RfidManager::init() {
    _pn532.begin();

    uint32_t versiondata = 0;
    for (int i = 0; i < 3; i++) {
        versiondata = _pn532.getFirmwareVersion();
        if (versiondata) break;
        LOG_WARN("PN532 not found. Retrying in 500ms...");
        delay(500);
    }

    if (!versiondata) {
        if (CFG_DEBUG_MODE) {
            LOG_WARN("PN532 RFID reader not found over SPI (version: 0x00)");
            LOG_WARN("Enabling Serial/Simulated RFID mode (Type a UID in Serial Monitor to scan!)");
            _initialized = true;
            _isSimulated = true;
        } else {
            LOG_ERROR("CRITICAL: PN532 RFID reader not found over SPI! Halting initialization.");
            _initialized = false;
        }
        return;
    }

    uint8_t chip   = (versiondata >> 24) & 0xFF;
    uint8_t ver    = (versiondata >> 16) & 0xFF;
    uint8_t rev    = (versiondata >> 8) & 0xFF;

    LOG_INFO("STM32 PN532 RFID Manager initialized!");
    LOG_INFO("  Found chip PN5%02X (Firmware v%d.%d)", chip, ver, rev);

    _pn532.SAMConfig();

    _initialized = true;
    _isSimulated = false;
}

void STM32RfidManager::update() {
    _newCardDetected = false;

    if (!_initialized || _isSimulated) return;

    unsigned long now = millis();
    if (now - _lastPollMs < RFID_POLL_INTERVAL_MS) {
        return;
    }
    _lastPollMs = now;

    uint8_t uidBuffer[7];
    uint8_t uidLength = 0;

    bool success = _pn532.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuffer, &uidLength, 50);

    if (success && uidLength > 0) {
        String currentUID = Utils::uidToHexString(uidBuffer, uidLength);

        if (_cardPresent && currentUID == _previousUID) {
            _lastReadMs = now;
            return;
        }

        if (now - _lastReadMs < RFID_DEBOUNCE_MS && currentUID == _previousUID) {
            return;
        }

        _cardPresent     = true;
        _newCardDetected = true;
        _lastUID         = currentUID;
        _previousUID     = currentUID;
        _lastReadMs      = now;

        LOG_INFO("STM32 RFID Card detected! UID: %s (length: %d)", _lastUID.c_str(), uidLength);
    } else {
        _cardPresent = false;
    }
}

bool STM32RfidManager::cardDetected() {
    return _newCardDetected;
}

String STM32RfidManager::lastUID() {
    return _lastUID;
}

void STM32RfidManager::triggerSimulatedCard(const String& uid) {
    _newCardDetected = true;
    _lastUID         = uid;
    _previousUID     = uid;
    _lastReadMs      = millis();
    LOG_INFO("[SIMULATED] Card scanned: %s", uid.c_str());
}

void STM32RfidManager::processSimulatedInput(const String& input) {
    String trimmed = input;
    trimmed.trim();
    if (trimmed.length() >= 4 && trimmed.length() <= 16) {
        bool isValidHex = true;
        for (unsigned int i = 0; i < trimmed.length(); i++) {
            if (!isxdigit(trimmed.charAt(i))) {
                isValidHex = false;
                break;
            }
        }
        if (isValidHex) {
            trimmed.toUpperCase();
            triggerSimulatedCard(trimmed);
        }
    }
}

#endif // ARDUINO_ARCH_STM32

