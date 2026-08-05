#pragma once
// ============================================================================
// buzzer_manager.h — Non-blocking Buzzer Tone Patterns
// ============================================================================

#include <Arduino.h>
#include "constants.h"
#include "IBuzzer.h"

class BuzzerManager : public IBuzzer {
public:
    void init() override;
    void update() override;  // Call every loop iteration

    void beepSuccess() override;      // 1 short high beep
    void beepDuplicate() override;    // 2 short beeps
    void beepUnknown() override;      // 1 long low beep
    void beepNetworkError() override; // 3 short low beeps
    void beepError() override { beepUnknown(); }

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
