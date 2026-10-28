#pragma once
// ============================================================================
// led_manager.h — STM32 Non-blocking LED Blink Patterns
// ============================================================================

#include <Arduino.h>
#include "constants.h"
#include "ILed.h"

class STM32LedManager : public ILed {
public:
    void init() override;
    void update() override;

    void blinkSuccess() override;
    void blinkDuplicate() override;
    void blinkFailure() override;

    void greenOn() override;
    void greenOff() override;
    void redOn() override;
    void redOff() override;
    void allOff() override;

private:
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
