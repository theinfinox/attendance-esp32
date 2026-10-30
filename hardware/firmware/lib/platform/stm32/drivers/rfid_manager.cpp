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
    : _pn532(PIN_RFID_SDA, &SPI)
{
}

void STM32RfidManager::init() {
    // 1. Cold boot settling delay for 27.12MHz crystal and 3.3V LDO on PN532
    delay(200);

    // 2. Drive RST HIGH in case physical RSTPD_N line is wired
    pinMode(PIN_RFID_RST, OUTPUT);
    digitalWrite(PIN_RFID_RST, HIGH);

    // 3. Initialize Hardware SPI so SCK (PA5) and MOSI (PA7) are actively driven
    SPI.begin();

    // 4. Hardware SPI wake-up pulse per NXP UM0701-02 §6.2.2.1
    pinMode(PIN_RFID_SDA, OUTPUT);
    digitalWrite(PIN_RFID_SDA, HIGH);
    delay(20);
    digitalWrite(PIN_RFID_SDA, LOW);   // Wake pulse start (T_wake >= 1ms)
    delay(5);
    digitalWrite(PIN_RFID_SDA, HIGH);  // Wake pulse end
    delay(15);                         // Allow PLL / oscillator lock

    // 5. Comprehensive 4-Pin Electrical Profile (SCK/PA5, MISO/PA6, MOSI/PA7, SS/PB6)
    pinMode(PIN_RFID_SCK, INPUT_PULLDOWN);
    pinMode(PIN_RFID_MISO, INPUT_PULLDOWN);
    pinMode(PIN_RFID_MOSI, INPUT_PULLDOWN);
    pinMode(PIN_RFID_SDA, INPUT_PULLDOWN);
    delay(2);
    int pd_sck  = digitalRead(PIN_RFID_SCK);
    int pd_miso = digitalRead(PIN_RFID_MISO);
    int pd_mosi = digitalRead(PIN_RFID_MOSI);
    int pd_ss   = digitalRead(PIN_RFID_SDA);

    pinMode(PIN_RFID_SCK, INPUT_PULLUP);
    pinMode(PIN_RFID_MISO, INPUT_PULLUP);
    pinMode(PIN_RFID_MOSI, INPUT_PULLUP);
    pinMode(PIN_RFID_SDA, INPUT_PULLUP);
    delay(2);
    int pu_sck  = digitalRead(PIN_RFID_SCK);
    int pu_miso = digitalRead(PIN_RFID_MISO);
    int pu_mosi = digitalRead(PIN_RFID_MOSI);
    int pu_ss   = digitalRead(PIN_RFID_SDA);

    LOG_INFO("Pin Profile (PULLDOWN): SCK(D13)=%d, MISO(D12)=%d, MOSI(D11)=%d, SS(D10)=%d",
             pd_sck, pd_miso, pd_mosi, pd_ss);
    LOG_INFO("Pin Profile (PULLUP):   SCK(D13)=%d, MISO(D12)=%d, MOSI(D11)=%d, SS(D10)=%d",
             pu_sck, pu_miso, pu_mosi, pu_ss);

    // 5b. Software SPI Direct Bit-Bang Probe (100 kHz, Mode 0, LSBFIRST)
    pinMode(PIN_RFID_SDA, OUTPUT);  // SS (PB6 / D10)
    pinMode(PIN_RFID_SCK, OUTPUT);  // SCK (PA5 / D13)
    pinMode(PIN_RFID_MOSI, OUTPUT); // MOSI (PA7 / D11)
    pinMode(PIN_RFID_MISO, INPUT);  // MISO (PA6 / D12)

    digitalWrite(PIN_RFID_SDA, HIGH);
    digitalWrite(PIN_RFID_SCK, LOW);
    digitalWrite(PIN_RFID_MOSI, LOW);
    delay(5);

    // Pulse NSS LOW to select PN532
    digitalWrite(PIN_RFID_SDA, LOW);
    delayMicroseconds(10);

    // Send 0x02 (STATREAD) bit by bit (LSBFIRST)
    uint8_t statCmd = 0x02;
    for (int i = 0; i < 8; i++) {
        digitalWrite(PIN_RFID_MOSI, (statCmd >> i) & 1);
        delayMicroseconds(5);
        digitalWrite(PIN_RFID_SCK, HIGH);
        delayMicroseconds(5);
        digitalWrite(PIN_RFID_SCK, LOW);
    }

    // Read status reply byte (8 clocks)
    uint8_t swReply = 0;
    for (int i = 0; i < 8; i++) {
        delayMicroseconds(5);
        digitalWrite(PIN_RFID_SCK, HIGH);
        delayMicroseconds(5);
        if (digitalRead(PIN_RFID_MISO)) {
            swReply |= (1 << i);
        }
        digitalWrite(PIN_RFID_SCK, LOW);
    }
    digitalWrite(PIN_RFID_SDA, HIGH);
    LOG_INFO("Software SPI Probe -> STATREAD reply: 0x%02X", swReply);

    // Re-initialize hardware SPI for Adafruit library
    SPI.begin();

    // 6. Begin PN532 library
    _pn532.begin();

    // Release NSS HIGH so falling edge synchronizes subsequent SPI commands
    digitalWrite(PIN_RFID_SDA, HIGH);
    delay(5);

    uint32_t versiondata = 0;
    for (int i = 0; i < 10; i++) {
        digitalWrite(PIN_RFID_SDA, LOW);
        delay(2);
        digitalWrite(PIN_RFID_SDA, HIGH);
        delay(5);

        versiondata = _pn532.getFirmwareVersion();
        if (versiondata) break;
        LOG_WARN("PN532 not found. Retrying in 250ms...");
        delay(250);
    }

    if (!versiondata) {
        if (CFG_DEBUG_MODE) {
            LOG_WARN("PN532 RFID reader not found over SPI (version: 0x00)");
            LOG_WARN("Enabling Serial/Simulated RFID mode (Will auto-reconnect if PN532 is plugged in)");
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

    if (!_initialized || _isSimulated) {
        static unsigned long lastRetryMs = 0;
        unsigned long now = millis();
        if (now - lastRetryMs > 1500) {
            lastRetryMs = now;
            pinMode(PIN_RFID_SDA, OUTPUT);
            digitalWrite(PIN_RFID_SDA, LOW);
            delay(2);
            digitalWrite(PIN_RFID_SDA, HIGH);
            delay(5);

            uint32_t v = _pn532.getFirmwareVersion();
            if (v) {
                _pn532.SAMConfig();
                _initialized = true;
                _isSimulated = false;
                LOG_INFO("STM32 PN532 RFID reader reconnected & online!");
            }
        }
        return;
    }

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
        if (_cardPresent && (now - _lastReadMs >= RFID_DEBOUNCE_MS)) {
            _cardPresent = false;
            _previousUID = "";
        }
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

