#pragma once
// ============================================================================
// rfid_manager.h — STM32 PN532 RFID Reader Manager
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
    bool           _initialized      = false;
    bool           _isSimulated      = false;
    bool           _newCardDetected  = false;
    String         _lastUID          = "";
    String         _previousUID      = "";
    unsigned long  _lastReadMs       = 0;
    unsigned long  _lastPollMs       = 0;
    bool           _cardPresent      = false;
};
