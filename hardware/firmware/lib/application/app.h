#pragma once
// ============================================================================
// app.h — Application Orchestrator
// Top-level class coordinating all managers and the state machine.
// ============================================================================

#include <Arduino.h>
#include "types.h"
#include "state_machine.h"
#include "IRFID.h"
#include "IDisplay.h"
#include "ILed.h"
#include "IBuzzer.h"
#include "IAttendanceRepository.h"
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
#include "wifi_manager.h"
#endif
#include "api_client.h"

class App {
public:
    void init(IDisplay* display, IRFID* rfid, ILed* led, IBuzzer* buzzer, IAttendanceRepository* repo);
    void update();  // Called from loop()
    SystemState getState() const { return _sm.currentState(); }

private:
    // --- Managers ---
    StateMachine           _sm;
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    WifiManager            _wifi;
#endif
    ApiClient              _api;
    
    // --- Interfaces ---
    IDisplay*              _display = nullptr;
    IRFID*                 _rfid = nullptr;
    ILed*                  _led = nullptr;
    IBuzzer*               _buzzer = nullptr;
    IAttendanceRepository* _repo = nullptr;

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
