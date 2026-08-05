// ============================================================================
// main.cpp — Entry Point
// Minimal setup/loop delegating entirely to App.
// ============================================================================

#include <Arduino.h>
#include "constants.h"
#include "app.h"

App app;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);
    app.init();
}

void loop() {
    app.update();
}
