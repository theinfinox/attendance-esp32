#pragma once
// ============================================================================
// rfid_manager.h — PN532 RFID Reader Manager
// Supports PN532 (Red board) over SPI with non-blocking card detection,
// duplicate prevention, and Serial monitor simulation fallback.
// ============================================================================

#include <Arduino.h>
#include <Adafruit_PN532.h>
#include "pins.h"
#include "constants.h"

class RfidManager {
public:
    RfidManager();

    void init();
    void update();  // Call every loop iteration

    // Was a new card just detected this cycle?
    bool cardDetected() const { return _newCardDetected; }

    // Get the UID of the last detected card (uppercase hex)
    const String& lastUID() const { return _lastUID; }

    // Is the reader initialized and functional?
    bool ready() const { return _initialized; }

    // Is running in simulated/serial mode?
    bool isSimulated() const { return _isSimulated; }

    // Manually trigger a card scan (useful for testing/simulation)
    void triggerSimulatedCard(const String& uid = "A1B2C3D4");

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
