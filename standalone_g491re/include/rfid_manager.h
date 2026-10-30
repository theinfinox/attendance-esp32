#pragma once
// ============================================================================
// rfid_manager.h — STM32 PN532 RFID Reader Manager
// Multi-layer Duplicate Protection: 2s debounce, 5s same-UID cooldown,
// physical card removal verification, boot-time card-presence validation.
// ============================================================================

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "constants.h"
#include "IRFID.h"

class STM32RfidManager : public IRFID {
public:
    STM32RfidManager();

    void init() override;
    void update() override;

    bool cardDetected() override;
    String lastUID() override;
    bool ready() const override { return _initialized; }
    bool isSimulated() const { return _isSimulated; }

    void triggerSimulatedCard(const String& uid = "A1B2C3D4");
    void processSimulatedInput(const String& input);

private:
    Adafruit_PN532 _pn532;
    bool           _initialized           = false;
    bool           _isSimulated           = false;
    bool           _newCardDetected       = false;
    String         _lastUID               = "";
    String         _activePresentedUID    = "";
    unsigned long  _lastScanTimeMs        = 0;
    unsigned long  _lastPollMs            = 0;
    bool           _cardPresent           = false;
    uint8_t        _consecutiveNoCard     = 0;

    struct UidCooldown {
        char uid[16];
        unsigned long lastAcceptedMs;
    };
    UidCooldown _cooldowns[8];
    size_t      _cooldownCount            = 0;

    unsigned long getUidCooldown(const String& uid);
    void setUidCooldown(const String& uid, unsigned long now);
};
