// ============================================================================
// rfid_manager.cpp — PN532 RFID Reader Manager (Software SPI)
// ============================================================================

#include "rfid_manager.h"
#include "utils.h"
#include "logger.h"
#include "pins.h"

RfidManager::RfidManager()
    : _pn532(PIN_RFID_SCK, PIN_RFID_MISO, PIN_RFID_MOSI, PIN_RFID_SDA)
{
}

void RfidManager::init() {
    _pn532.begin();

    // Check PN532 firmware version (with retries for loose connections)
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

    // Got version data!
    uint8_t chip   = (versiondata >> 24) & 0xFF;
    uint8_t ver    = (versiondata >> 16) & 0xFF;
    uint8_t rev    = (versiondata >> 8) & 0xFF;

    LOG_INFO("PN532 RFID Manager initialized!");
    LOG_INFO("  Found chip PN5%02X (Firmware v%d.%d)", chip, ver, rev);

    // Configure board to read RFID tags (SAM = Secure Access Module)
    _pn532.SAMConfig();

    _initialized = true;
    _isSimulated = false;
}

void RfidManager::update() {
    _newCardDetected = false;

    if (!_initialized || _isSimulated) return;

    // Throttle hardware polling
    unsigned long now = millis();
    if (now - _lastPollMs < RFID_POLL_INTERVAL_MS) return;
    _lastPollMs = now;

    // Read card with a short 50ms timeout for non-blocking operation
    uint8_t success = 0;
    uint8_t uid[7]  = {0};
    uint8_t uidLen  = 0;

    success = _pn532.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 50);

    if (!success) {
        // No card present — reset duplicate prevention after debounce
        if (_cardPresent && (now - _lastReadMs >= RFID_DEBOUNCE_MS)) {
            _cardPresent  = false;
            _previousUID  = "";
            LOG_DEBUG("RFID: card removed (debounce expired)");
        }
        return;
    }

    // Convert raw UID bytes to uppercase hex string
    String cardUID = Utils::uidToHexString(uid, uidLen);
    if (cardUID.isEmpty()) return;

    // Duplicate prevention: same card still on reader
    if (_cardPresent && cardUID == _previousUID) {
        _lastReadMs = now;  // Reset debounce timer
        return;
    }

    // New card detected!
    _lastUID         = cardUID;
    _previousUID     = cardUID;
    _cardPresent     = true;
    _newCardDetected = true;
    _lastReadMs      = now;

    LOG_INFO("PN532: Card detected UID=%s", cardUID.c_str());
}

void RfidManager::triggerSimulatedCard(const String& uid) {
    _lastUID         = uid;
    _newCardDetected = true;
    _lastReadMs      = millis();
    LOG_INFO("RFID [SIMULATED]: Card scanned UID=%s", uid.c_str());
}

void RfidManager::processSimulatedInput(const String& inputStr) {
    if (!CFG_DEBUG_MODE) return;
    String input = inputStr;
    input.trim();
    if (input.length() > 0) {
        if (input.startsWith("SCAN") || input.startsWith("scan")) {
            int spaceIdx = input.indexOf(' ');
            String uid = (spaceIdx != -1) ? input.substring(spaceIdx + 1) : "A1B2C3D4";
            uid.trim();
            uid.toUpperCase();
            triggerSimulatedCard(uid);
        } else if (input.length() >= 4 && input.length() <= 20) {
            input.toUpperCase();
            triggerSimulatedCard(input);
        }
    }
}

bool RfidManager::cardDetected() {
    return _newCardDetected;
}

String RfidManager::lastUID() {
    return _lastUID;
}
