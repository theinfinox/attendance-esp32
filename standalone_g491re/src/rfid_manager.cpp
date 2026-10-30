// ============================================================================
// rfid_manager.cpp — Enhanced STM32 PN532 Hardware SPI Driver & Recovery
// Multi-layer Duplicate Protection: 2s debounce, 5s same-UID cooldown,
// physical card removal verification, boot-time card-presence validation.
// ============================================================================

#include "rfid_manager.h"
#include "logger.h"
#include "pins.h"

STM32RfidManager::STM32RfidManager()
    : _pn532(PIN_RFID_SDA, &SPI) {}

unsigned long STM32RfidManager::getUidCooldown(const String& uid) {
    for (size_t i = 0; i < _cooldownCount; ++i) {
        if (strcasecmp(_cooldowns[i].uid, uid.c_str()) == 0) {
            return _cooldowns[i].lastAcceptedMs;
        }
    }
    return 0;
}

void STM32RfidManager::setUidCooldown(const String& uid, unsigned long now) {
    for (size_t i = 0; i < _cooldownCount; ++i) {
        if (strcasecmp(_cooldowns[i].uid, uid.c_str()) == 0) {
            _cooldowns[i].lastAcceptedMs = now;
            return;
        }
    }
    if (_cooldownCount < 8) {
        strncpy(_cooldowns[_cooldownCount].uid, uid.c_str(), sizeof(_cooldowns[_cooldownCount].uid) - 1);
        _cooldowns[_cooldownCount].lastAcceptedMs = now;
        _cooldownCount++;
    } else {
        // Round-robin replace slot 0
        strncpy(_cooldowns[0].uid, uid.c_str(), sizeof(_cooldowns[0].uid) - 1);
        _cooldowns[0].lastAcceptedMs = now;
    }
}

void STM32RfidManager::init() {
    LOG_INFO("STM32: Initializing PN532 RFID Manager on Hardware SPI1...");

    // 1. Ensure SPI1 bus pins are in a known state
    pinMode(PIN_RFID_SDA, OUTPUT);
    digitalWrite(PIN_RFID_SDA, HIGH);

    #ifdef PIN_RFID_RST
    pinMode(PIN_RFID_RST, OUTPUT);
    digitalWrite(PIN_RFID_RST, HIGH);
    #endif

    // 2. Explicit NXP UM0701 SPI Wake-Up Sequence
    delay(10);
    digitalWrite(PIN_RFID_SDA, LOW);
    delay(3);                          // Meet NXP >1ms wake requirement
    digitalWrite(PIN_RFID_SDA, HIGH);
    delay(10);                         // Wait for 27.12MHz crystal lock

    // 3. Initialize Hardware SPI & PN532
    _pn532.begin();

    // 4. Query Firmware Version with up to 3 retries
    uint32_t versiondata = 0;
    for (int retry = 0; retry < 3; ++retry) {
        versiondata = _pn532.getFirmwareVersion();
        if (versiondata) break;
        
        LOG_WARN("STM32: PN532 detection attempt %d failed, pulsing NSS...", retry + 1);
        digitalWrite(PIN_RFID_SDA, LOW);
        delay(2);
        digitalWrite(PIN_RFID_SDA, HIGH);
        delay(5);
    }

    if (!versiondata) {
        LOG_WARN("STM32: PN532 not detected on SPI1! Activating SIMULATED fallback.");
        _initialized = true;
        _isSimulated = true;
        return;
    }

    uint8_t chip = (versiondata >> 24) & 0xFF;
    uint8_t fwVer = (versiondata >> 16) & 0xFF;
    uint8_t fwRev = (versiondata >> 8) & 0xFF;
    uint8_t support = versiondata & 0xFF;

    LOG_INFO("STM32: Found PN532 (0x%02X) Firmware v%d.%d (Support=0x%02X)", chip, fwVer, fwRev, support);

    _pn532.SAMConfig();
    _pn532.setPassiveActivationRetries(0x01); // Non-blocking polling

    // 5. Boot-time card-presence validation
    // If a card is resting on the antenna during power-up, latch it as present
    // so it does NOT fire an attendance event until lifted and removed!
    uint8_t bootUid[7];
    uint8_t bootLen = 0;
    if (_pn532.readPassiveTargetID(PN532_MIFARE_ISO14443A, bootUid, &bootLen, 50) && bootLen > 0) {
        _cardPresent = true;
        _consecutiveNoCard = 0;
        String bUid = "";
        for (uint8_t i = 0; i < bootLen; ++i) {
            if (bootUid[i] < 0x10) bUid += "0";
            bUid += String(bootUid[i], HEX);
        }
        bUid.toUpperCase();
        _activePresentedUID = bUid;
        LOG_INFO("STM32: Card (%s) resting on antenna at boot. Remove card before scanning.", bUid.c_str());
    }

    _initialized = true;
    _isSimulated = false;
    LOG_INFO("STM32: PN532 RFID Manager initialized successfully. [ONLINE]");
}

void STM32RfidManager::update() {
    if (!_initialized) return;

    unsigned long now = millis();

    // Periodic Reconnect Recovery
    if (_isSimulated && (now - _lastPollMs >= 5000)) {
        _lastPollMs = now;
        uint32_t ver = _pn532.getFirmwareVersion();
        if (ver) {
            LOG_INFO("STM32: PN532 reconnected at runtime! Recovering to ONLINE mode.");
            _pn532.SAMConfig();
            _pn532.setPassiveActivationRetries(0x01);
            _isSimulated = false;
        }
        return;
    }

    if (now - _lastPollMs < RFID_POLL_INTERVAL_MS) {
        return;
    }
    _lastPollMs = now;

    if (_isSimulated) {
        return;
    }

    // Hardware Non-Blocking Card Scan
    uint8_t uidBuffer[7];
    uint8_t uidLength = 0;

    bool cardDetectedInField = _pn532.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuffer, &uidLength, 50);

    if (!cardDetectedInField || uidLength == 0) {
        _consecutiveNoCard++;
        // Card must be absent for at least 2 consecutive cycles (>=400ms) to confirm removal
        if (_consecutiveNoCard >= 2) {
            _cardPresent = false;
            _activePresentedUID = "";
        }
        return;
    }

    // Card detected in RF field
    _consecutiveNoCard = 0;

    String currentUID = "";
    for (uint8_t i = 0; i < uidLength; ++i) {
        if (uidBuffer[i] < 0x10) currentUID += "0";
        currentUID += String(uidBuffer[i], HEX);
    }
    currentUID.toUpperCase();

    // Layer 1: Physical Card Removal Verification
    // A card held continuously in the field will NEVER alternate IN and OUT!
    if (_cardPresent && currentUID == _activePresentedUID) {
        return;
    }

    _cardPresent = true;
    _activePresentedUID = currentUID;

    // Layer 2: Minimum scan debounce (2000 ms across any scan)
    if (now - _lastScanTimeMs < RFID_DEBOUNCE_MS) {
        return;
    }

    // Layer 3: Same-UID Duplicate Suppression (5000 ms)
    unsigned long uidCooldownMs = getUidCooldown(currentUID);
    if (now - uidCooldownMs < 5000) {
        LOG_WARN("STM32: Duplicate scan suppressed for UID=%s (<5s cooldown)", currentUID.c_str());
        return;
    }

    // Accepted!
    _lastScanTimeMs = now;
    setUidCooldown(currentUID, now);
    _lastUID = currentUID;
    _newCardDetected = true;
    LOG_INFO("STM32: Accepted card scan UID=%s (Length=%d)", _lastUID.c_str(), uidLength);
}

bool STM32RfidManager::cardDetected() {
    if (_newCardDetected) {
        _newCardDetected = false;
        return true;
    }
    return false;
}

String STM32RfidManager::lastUID() {
    return _lastUID;
}

void STM32RfidManager::triggerSimulatedCard(const String& uid) {
    unsigned long now = millis();
    String cleanUid = uid; cleanUid.trim(); cleanUid.toUpperCase();

    // Check same-UID 5s cooldown even for simulated inputs
    unsigned long cooldown = getUidCooldown(cleanUid);
    if (now - cooldown < 5000) {
        LOG_WARN("STM32: Simulated duplicate scan suppressed for UID=%s (<5s cooldown)", cleanUid.c_str());
        return;
    }

    setUidCooldown(cleanUid, now);
    _lastUID = cleanUid;
    _newCardDetected = true;
    LOG_INFO("STM32: Simulated Card Scanned: %s", _lastUID.c_str());
}

void STM32RfidManager::processSimulatedInput(const String& input) {
    String clean = input;
    clean.trim();
    if (clean.length() >= 4) {
        triggerSimulatedCard(clean);
    }
}
