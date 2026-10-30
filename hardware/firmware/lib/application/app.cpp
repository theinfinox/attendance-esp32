// ============================================================================
// app.cpp — Application Orchestrator
// ============================================================================

#include "app.h"
#include "logger.h"
#include "version.h"
#include "app_utils.h"
#include "constants.h"
#include <time.h>

static String getFormattedTime() {
    time_t t;
    time(&t);
    struct tm* timeinfo = localtime(&t);
    // Basic check to see if time is synced (year > 2000)
    if (timeinfo->tm_year > 100) {
        char buf[10];
        snprintf(buf, sizeof(buf), "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
        return String(buf);
    }
    return "";
}

// ---- Init ------------------------------------------------------------------

void App::init(IDisplay* display, IRFID* rfid, ILed* led, IBuzzer* buzzer, IAttendanceRepository* repo) {
    _display = display;
    _rfid = rfid;
    _led = led;
    _buzzer = buzzer;
    _repo = repo;
    LOG_INFO("=================================");
    LOG_INFO("  %s", FW_PROJECT_NAME);
    LOG_INFO("  Firmware v%s", FW_VERSION);
    LOG_INFO("  Built: %s %s", FW_BUILD_DATE, FW_BUILD_TIME);
    LOG_INFO("  Device: %s", CFG_DEVICE_ID);
    LOG_INFO("=================================");

    // Initialize state machine first
    _sm.init();

    // Initialize display for boot animation
    _display->init();

    // Boot animation with progress bar
    for (uint8_t p = 0; p <= 100; p += 5) {
        _display->showBoot(p);
        delay(BOOT_ANIMATION_STEP_MS);
    }
    delay(500);

    // Transition to INIT
    _sm.transition(SystemState::INIT);
}

// ---- Update (main loop) ----------------------------------------------------

void App::update() {
    // Always update non-blocking managers
    _led->update();
    _buzzer->update();

    // Store state before execution to detect transitions triggered inside the handlers
    SystemState beforeState = _sm.currentState();

    // Run state-specific logic
    switch (_sm.currentState()) {
        case SystemState::BOOT:
            // Should not stay here — init() transitions out
            break;

        case SystemState::INIT:
            handleInit();
            break;

        case SystemState::WIFI_CONNECTING:
            handleWifiConnecting();
            break;

        case SystemState::BACKEND_CONNECTING:
            handleBackendConnecting();
            break;

        case SystemState::READY:
            handleReady();
            break;

        case SystemState::CARD_DETECTED:
            handleCardDetected();
            break;

        case SystemState::SENDING:
            handleSending();
            break;

        case SystemState::SUCCESS:
            handleSuccess();
            break;

        case SystemState::FAILED:
            handleFailed();
            break;

        case SystemState::NETWORK_ERROR:
            handleNetworkError();
            break;
    }

    // Only clear the state-changed flag if a handler didn't just trigger a new transition
    if (_sm.currentState() == beforeState) {
        _sm.clearStateChanged();
    }
}

// ---- State Handlers --------------------------------------------------------

void App::handleInit() {
    if (_sm.stateChanged()) {
        _display->showInit();
        LOG_INFO("Initializing hardware managers...");

        // Initialize all managers
        _led->init();
        _buzzer->init();
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
        _wifi.init();
#endif
        _rfid->init();
        _repo->init();
        _api.init(_repo);

        if (!_rfid->ready()) {
            LOG_ERROR("RFID reader failed to initialize!");
            _display->showShutdown("RFID Init Failed");
            _led->blinkFailure();
            _buzzer->beepNetworkError();
            // Stay in INIT — won't proceed without RFID
            return;
        }

        LOG_INFO("All hardware initialized OK");
        LOG_INFO("Free heap: %u bytes", Utils::freeHeap());

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
        // Proceed to WiFi connection
        _sm.transition(SystemState::WIFI_CONNECTING);
#else
        LOG_INFO("Operating in standalone offline/serial mode — system READY");
        _sm.transition(SystemState::READY);
#endif
    }
}

void App::handleWifiConnecting() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    if (_sm.stateChanged()) {
        _wifiAnimCounter = 0;
        _lastWifiAnimMs = 0;
        _wifi.connect();
    }

    // Update WiFi manager
    _wifi.update();

    // Animate the display
    unsigned long now = millis();
    if (now - _lastWifiAnimMs >= 500) {
        _lastWifiAnimMs = now;
        _wifiAnimCounter++;
        _display->showWiFiConnecting(_wifi.connectionAttempt());
    }

    // Check if connected
    if (_wifi.justConnected()) {
        LOG_INFO("WiFi connected — proceeding to backend check");
        _sm.transition(SystemState::BACKEND_CONNECTING);
    }

    // Timeout after many attempts — proceed in OFFLINE mode
    if (_sm.timeInState() > WIFI_CONNECT_TIMEOUT_MS * 3) {
        LOG_WARN("WiFi failed after extended timeout. Proceeding in OFFLINE mode.");
        _sm.transition(SystemState::READY);
    }
#else
    _sm.transition(SystemState::READY);
#endif
}

void App::handleBackendConnecting() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    if (_sm.stateChanged()) {
        _display->showBackendConnecting();
    }

    // Update WiFi (keep monitoring)
    _wifi.update();

    if (_wifi.justDisconnected()) {
        _sm.transition(SystemState::NETWORK_ERROR);
        return;
    }

    // Ping backend
    bool backendOk = _api.ping();

    if (backendOk) {
        LOG_INFO("Backend reachable — system READY");
        _led->greenOn();
        _sm.transition(SystemState::READY);
    } else {
        // For mocked mode, ping always returns true
        // For real mode, retry or show error
        LOG_WARN("Backend ping failed — retrying...");
        if (_sm.timeInState() > API_REQUEST_TIMEOUT_MS * API_MAX_RETRIES) {
            _sm.transition(SystemState::FAILED);
        }
    }
#else
    _sm.transition(SystemState::READY);
#endif
}

void App::handleReady() {
    static String lastTimeStr = "";
    String currentTimeStr = getFormattedTime();

    if (_sm.stateChanged() || currentTimeStr != lastTimeStr) {
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
        _display->showReady(_wifi.ip(), _wifi.rssi(), currentTimeStr);
#else
        String rfidStatus = (_rfid->ready() && !_rfid->isSimulated()) ? "PN532 Online" : "RFID: Offline/Sim";
        _display->showReady(rfidStatus, 0, currentTimeStr);
#endif
        if (_sm.stateChanged()) {
            _led->allOff();
            _led->greenOn();
        }
        lastTimeStr = currentTimeStr;
    }

    // Update managers
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    _wifi.update();
#endif
    _rfid->update();

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    // Check WiFi
    if (_wifi.justDisconnected()) {
        _led->allOff();
        _sm.transition(SystemState::NETWORK_ERROR);
        return;
    }
#endif

    // Check for RFID card
    if (_rfid->cardDetected()) {
        _currentUID = _rfid->lastUID();
        _led->greenOff();
        _sm.transition(SystemState::CARD_DETECTED);
    }

    // Periodically refresh display with updated RSSI and Time
    unsigned long now = millis();
    if (now - _lastRssiUpdateMs >= 1000) { // Update every 1s for the clock
        _lastRssiUpdateMs = now;
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
        _display->showReady(_wifi.ip(), _wifi.rssi(), getFormattedTime());
#else
        String rfidStatus = (_rfid->ready() && !_rfid->isSimulated()) ? "PN532 Online" : "RFID: Offline/Sim";
        _display->showReady(rfidStatus, 0, getFormattedTime());
#endif
    }
}

void App::handleCardDetected() {
    if (_sm.stateChanged()) {
        if (CFG_SHOW_UID_ON_DETECT) {
            _display->showCardDetected(_currentUID);
        } else {
            _display->showCardDetected("");
        }
        _buzzer->beepSuccess();  // Short acknowledgment beep
        LOG_INFO("Processing card UID: %s", _currentUID.c_str());
    }

    // Brief pause to show the card detected screen on OLED, then send
    if (_sm.timeInState() >= 800) {
        _sm.transition(SystemState::SENDING);
    }
}

void App::handleSending() {
    if (_sm.stateChanged()) {
        LOG_INFO("Submitting attendance for UID=%s", _currentUID.c_str());

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
        // Check WiFi before sending
        if (!CFG_STANDALONE_MODE && !_wifi.connected()) {
            LOG_WARN("Offline mode: Storing record locally");
            AttendanceResponse resp;
            resp.result = AttendanceResult::NETWORK_ERROR;
            resp.message = "Offline: Saved locally";
            processAttendanceResult(resp);
            return;
        }
#endif

        // Call API (mocked or real)
        AttendanceResponse resp = _api.submitAttendance(_currentUID);
        processAttendanceResult(resp);
    }
}

void App::handleSuccess() {
    if (_sm.stateChanged()) {
        // Display and feedback already set by processAttendanceResult
    }

    // Update non-blocking managers
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    _wifi.update();
#endif
    _rfid->update();

    // Auto-return to READY
    if (_sm.shouldAutoReturn()) {
        _sm.transition(SystemState::READY);
        return;
    }

    // If another card is scanned during result display, handle it
    if (_rfid->cardDetected()) {
        _currentUID = _rfid->lastUID();
        _sm.transition(SystemState::CARD_DETECTED);
    }
}

void App::handleFailed() {
    if (_sm.stateChanged()) {
        // Display and feedback already set by processAttendanceResult
    }

    // Update non-blocking managers
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    _wifi.update();
#endif
    _rfid->update();

    // Auto-return to READY
    if (_sm.shouldAutoReturn()) {
        _sm.transition(SystemState::READY);
        return;
    }

    // If another card is scanned during result display
    if (_rfid->cardDetected()) {
        _currentUID = _rfid->lastUID();
        _sm.transition(SystemState::CARD_DETECTED);
    }
}

void App::handleNetworkError() {
    if (_sm.stateChanged()) {
        _display->showNetworkError();
        _led->blinkFailure();
        _buzzer->beepNetworkError();
        LOG_ERROR("Network error — attempting recovery");
    }

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    // Keep trying to reconnect
    _wifi.update();

    if (_wifi.justConnected()) {
        LOG_INFO("WiFi recovered!");
        _sm.transition(SystemState::READY);
    }
#endif

    // Auto-return to READY after a delay so we can continue scanning offline
    if (_sm.timeInState() > DISPLAY_RESULT_DURATION_MS) {
        _sm.transition(SystemState::READY);
    }

    // Animate display periodically
    unsigned long now = millis();
    if (now - _lastWifiAnimMs >= 1000) {
        _lastWifiAnimMs = now;
        _display->showNetworkError();
    }
}

// ---- Helpers ---------------------------------------------------------------

void App::processAttendanceResult(const AttendanceResponse& resp) {
    LOG_INFO("Attendance result: %s — %s",
             resultToString(resp.result), resp.message.c_str());

    switch (resp.result) {
        case AttendanceResult::OK:
            _display->showSuccess(resp.name);
            _led->blinkSuccess();
            _buzzer->beepSuccess();
            _sm.transition(SystemState::SUCCESS);
            break;

        case AttendanceResult::DUPLICATE:
            _display->showDuplicate();
            _led->blinkDuplicate();
            _buzzer->beepDuplicate();
            _sm.transition(SystemState::FAILED);
            break;

        case AttendanceResult::UNKNOWN_CARD:
            _display->showUnknownCard(_currentUID);
            _led->blinkFailure();
            _buzzer->beepUnknown();
            _sm.transition(SystemState::FAILED);
            break;

        case AttendanceResult::NETWORK_ERROR:
            // Store for offline sync
            // NOTE: Offline sync buffering omitted for now via _repo since that's backend-specific,
            // but we can log offline if we add it to the repo interface later.
            // _repo->storeOfflineRecord(_currentUID, millis()); 
            _sm.transition(SystemState::NETWORK_ERROR);
            break;

        case AttendanceResult::BACKEND_ERROR:
            _display->showBackendError();
            _led->blinkFailure();
            _buzzer->beepNetworkError();
            _sm.transition(SystemState::FAILED);
            break;

        case AttendanceResult::AUTH_ERROR:
            _display->showAuthError();
            _led->blinkFailure();
            _buzzer->beepUnknown();
            _sm.transition(SystemState::FAILED);
            break;
    }
}
