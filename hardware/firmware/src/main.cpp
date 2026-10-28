// ============================================================================
// main.cpp — Entry Point
// Minimal setup/loop delegating entirely to App.
// ============================================================================

#include <Arduino.h>
#include "constants.h"
#include "app.h"
#include "cli_manager.h"

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include "display_manager.h"
#include "rfid_manager.h"
#include "led_manager.h"
#include "buzzer_manager.h"
#include "LittleFSAttendanceRepository.h"
#include "web_manager.h"

DisplayManager displayManager;
RfidManager rfidManager;
LedManager ledManager;
BuzzerManager buzzerManager;
LittleFSAttendanceRepository repo;
WebManager web;

#elif defined(ARDUINO_ARCH_STM32)
#include "display_manager.h"
#include "rfid_manager.h"
#include "led_manager.h"
#include "buzzer_manager.h"
#include "STM32AttendanceRepository.h"

STM32DisplayManager displayManager;
STM32RfidManager rfidManager;
STM32LedManager ledManager;
STM32BuzzerManager buzzerManager;
STM32AttendanceRepository repo;
#endif

App app;
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
    
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
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
#endif
}
