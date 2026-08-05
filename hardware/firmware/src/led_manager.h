#pragma once
// ============================================================================
// led_manager.h — Non-blocking LED Blink Patterns
// ============================================================================

#include <Arduino.h>
#include "pins.h"
#include "constants.h"

class LedManager {
public:
    void init();
    void update();  // Call every loop iteration

    void blinkSuccess();    // 1 short green blink
    void blinkDuplicate();  // 2 short red blinks
    void blinkFailure();    // 1 long red blink

    void greenOn();
    void greenOff();
    void redOn();
    void redOff();
    void allOff();

private:
    // Pattern sequencing
    enum class PatternState : uint8_t { IDLE, ON, GAP, DONE };

    struct BlinkPattern {
        int            pin         = -1;
        uint8_t        totalBlinks = 0;
        uint8_t        currentBlink = 0;
        unsigned long  onDuration  = 0;
        unsigned long  gapDuration = 0;
        unsigned long  timer       = 0;
        PatternState   state       = PatternState::IDLE;
    };

    BlinkPattern _pattern;

    void startPattern(int pin, uint8_t count, unsigned long onMs, unsigned long gapMs);
};
