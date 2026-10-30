// ============================================================================
// main.cpp — Standalone STM32G491RE Diagnostic & Attendance Terminal Entry
// ============================================================================

#include <Arduino.h>
#include "constants.h"
#include "app.h"
#include "cli_manager.h"
#include "display_manager.h"
#include "rfid_manager.h"
#include "led_manager.h"
#include "buzzer_manager.h"
#include "STM32FlashRepository.h"

STM32DisplayManager   displayManager;
STM32RfidManager      rfidManager;
STM32LedManager       ledManager;
STM32BuzzerManager    buzzerManager;
STM32FlashRepository  repo;

App app;
CliManager cli;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);
    app.init(&displayManager, &rfidManager, &ledManager, &buzzerManager, &repo);
    cli.init(&repo, &rfidManager);
}

void loop() {
    String unhandled = cli.update();
    if (unhandled.length() > 0) {
        rfidManager.processSimulatedInput(unhandled);
    }
    app.update();
}

