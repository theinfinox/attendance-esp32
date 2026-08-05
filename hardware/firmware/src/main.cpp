// ============================================================================
// main.cpp — Entry Point
// Minimal setup/loop delegating entirely to App.
// ============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "constants.h"
#include "app.h"
#include "display_manager.h"
#include "rfid_manager.h"
#include "led_manager.h"
#include "buzzer_manager.h"
#include "LittleFSAttendanceRepository.h"
#include "web_manager.h"
#include "cli_manager.h"

App app;
DisplayManager displayManager;
RfidManager rfidManager;
LedManager ledManager;
BuzzerManager buzzerManager;
LittleFSAttendanceRepository repo;
WebManager web;
CliManager cli;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);
    app.init(&displayManager, &rfidManager, &ledManager, &buzzerManager, &repo);
    cli.init(&repo);
}

void loop() {
    String unhandled = cli.update();
    if (unhandled.length() > 0) {
        rfidManager.processSimulatedInput(unhandled);
    }
    app.update();
    
    if (CFG_STANDALONE_MODE) {
        static bool webInitialized = false;
        if (!webInitialized && app.getState() != SystemState::BOOT && app.getState() != SystemState::INIT) {
            web.init(&repo, &rfidManager);
            webInitialized = true;
        }
        if (webInitialized) {
            web.update();
        }
    }
}
