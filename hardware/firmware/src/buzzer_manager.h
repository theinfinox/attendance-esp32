#pragma once
// ============================================================================
// buzzer_manager.h — Non-blocking Buzzer Tone Patterns
// ============================================================================

#include <Arduino.h>
#include "pins.h"
#include "constants.h"

class BuzzerManager {
public:
    void init();
    void update();  // Call every loop iteration

    void beepSuccess();      // 1 short high beep
    void beepDuplicate();    // 2 short beeps
    void beepUnknown();      // 1 long low beep
    void beepNetworkError(); // 3 short low beeps

    void silence();

private:
    enum class ToneState : uint8_t { IDLE, ON, GAP, DONE };

    struct TonePattern {
        uint8_t        totalBeeps   = 0;
        uint8_t        currentBeep  = 0;
        unsigned int   frequency    = 0;
        unsigned long  onDuration   = 0;
        unsigned long  gapDuration  = 0;
        unsigned long  timer        = 0;
        ToneState      state        = ToneState::IDLE;
    };

    TonePattern _pattern;

    void startPattern(unsigned int freq, uint8_t count, unsigned long onMs, unsigned long gapMs);
};
