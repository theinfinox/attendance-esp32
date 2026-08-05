#pragma once
// ============================================================================
// wifi_manager.h — WiFi Connection Manager
// Handles connect, reconnect, and status. Never calls API or display.
// ============================================================================

#include <Arduino.h>
#include "config.h"
#include "constants.h"

class WifiManager {
public:
    void init();
    void update();  // Call every loop iteration for auto-reconnect

    bool connected() const;
    int  rssi() const;
    String ip() const;
    String mac() const;

    // Connection control
    void connect();
    void disconnect();

    // Track connection attempts for display
    int connectionAttempt() const { return _attempt; }

    // Was connection just established this cycle?
    bool justConnected() const { return _justConnected; }

    // Was connection just lost this cycle?
    bool justDisconnected() const { return _justDisconnected; }

private:
    bool          _wasConnected     = false;
    bool          _justConnected    = false;
    bool          _justDisconnected = false;
    int           _attempt          = 0;
    unsigned long _lastReconnectMs  = 0;
    unsigned long _connectStartMs   = 0;
    bool          _connecting       = false;
};
