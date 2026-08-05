#pragma once
// ============================================================================
// app.h — Application Orchestrator
// Top-level class coordinating all managers and the state machine.
// ============================================================================

#include <Arduino.h>
#include "types.h"
#include "state_machine.h"
#include "wifi_manager.h"
#include "rfid_manager.h"
#include "display_manager.h"
#include "led_manager.h"
#include "buzzer_manager.h"
#include "api_client.h"
#include "storage.h"
#include "web_manager.h"

class App {
public:
    void init();    // Called from setup()
    void update();  // Called from loop()

private:
    // --- Managers ---
    StateMachine   _sm;
    WifiManager    _wifi;
    RfidManager    _rfid;
    DisplayManager _display;
    LedManager     _led;
    BuzzerManager  _buzzer;
    ApiClient      _api;
    Storage        _storage;
    WebManager     _web;

    // --- State handlers ---
    void handleBoot();
    void handleInit();
    void handleWifiConnecting();
    void handleBackendConnecting();
    void handleReady();
    void handleCardDetected();
    void handleSending();
    void handleSuccess();
    void handleFailed();
    void handleNetworkError();

    // --- Helpers ---
    void processAttendanceResult(const AttendanceResponse& resp);

    // --- Internal state ---
    String        _currentUID      = "";
    unsigned long _wifiAnimCounter = 0;
    unsigned long _lastWifiAnimMs  = 0;
    unsigned long _lastRssiUpdateMs= 0;
    bool          _bootComplete    = false;
};
