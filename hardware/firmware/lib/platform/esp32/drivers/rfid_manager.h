#pragma once
// ============================================================================
// rfid_manager.h — PN532 RFID Reader Manager
// Supports PN532 (Red board) over SPI with non-blocking card detection,
// duplicate prevention, and Serial monitor simulation fallback.
// ============================================================================

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "constants.h"
#include "IRFID.h"

class RfidManager : public IRFID {
public:
    RfidManager();

    void init() override;
    void update() override; // Polling function

    // Was a new card just detected this cycle?
    bool cardDetected() override;

    // Get the UID of the last detected card (uppercase hex)
    String lastUID() override;

    // Is the reader initialized and functional?
    bool ready() const { return _initialized; }

    // Is running in simulated/serial mode?
    bool isSimulated() const { return _isSimulated; }

    // Manually trigger a card scan (useful for testing/simulation)
    void triggerSimulatedCard(const String& uid = "A1B2C3D4");
    void processSimulatedInput(const String& input);

private:
    Adafruit_PN532 _pn532;
    bool           _initialized      = false;
    bool           _isSimulated      = false;
    bool           _newCardDetected   = false;
    String         _lastUID           = "";
    String         _previousUID       = "";
    unsigned long  _lastReadMs        = 0;
    unsigned long  _lastPollMs        = 0;
    bool           _cardPresent       = false;
};
