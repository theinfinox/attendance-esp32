#if defined(ARDUINO_ARCH_STM32)
// ============================================================================
// buzzer_manager.cpp — STM32 Tone Implementation
// ============================================================================

#include "buzzer_manager.h"
#include "logger.h"
#include "pins.h"

static void playTone(int pin, unsigned int freq) {
    if (!BUZZER_ENABLED || freq == 0) return;
    tone(pin, freq);
}

static void stopTone(int pin) {
    if (!BUZZER_ENABLED) return;
    noTone(pin);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void STM32BuzzerManager::init() {
    if (!BUZZER_ENABLED) {
        LOG_INFO("STM32 Buzzer Manager initialized (DISABLED via constants.h)");
        return;
    }
    pinMode(PIN_BUZZER, OUTPUT);
    stopTone(PIN_BUZZER);
    silence();
    LOG_INFO("STM32 Buzzer Manager initialized");
}

void STM32BuzzerManager::update() {
    if (!BUZZER_ENABLED) return;
    if (_pattern.state == ToneState::IDLE || _pattern.state == ToneState::DONE) {
        return;
    }

    unsigned long now = millis();

    switch (_pattern.state) {
        case ToneState::ON:
            if (now - _pattern.timer >= _pattern.onDuration) {
                stopTone(PIN_BUZZER);
                _pattern.currentBeep++;

                if (_pattern.currentBeep >= _pattern.totalBeeps) {
                    _pattern.state = ToneState::DONE;
                } else {
                    _pattern.state = ToneState::GAP;
                    _pattern.timer = now;
                }
            }
            break;

        case ToneState::GAP:
            if (now - _pattern.timer >= _pattern.gapDuration) {
                playTone(PIN_BUZZER, _pattern.frequency);
                _pattern.state = ToneState::ON;
                _pattern.timer = now;
            }
            break;

        default:
            break;
    }
}

void STM32BuzzerManager::silence() {
    _pattern.state = ToneState::IDLE;
    stopTone(PIN_BUZZER);
}

void STM32BuzzerManager::startPattern(unsigned int freq, uint8_t count,
                                      unsigned long onMs, unsigned long gapMs) {
    if (!BUZZER_ENABLED) return;
    silence();
    _pattern.frequency    = freq;
    _pattern.totalBeeps   = count;
    _pattern.currentBeep  = 0;
    _pattern.onDuration   = onMs;
    _pattern.gapDuration  = gapMs;
    _pattern.timer        = millis();
    _pattern.state        = ToneState::ON;

    playTone(PIN_BUZZER, freq);
}

void STM32BuzzerManager::beepSuccess() {
    startPattern(BUZZER_FREQ_SUCCESS, 1, BUZZER_SHORT_MS, BUZZER_GAP_MS);
}

void STM32BuzzerManager::beepDuplicate() {
    startPattern(BUZZER_FREQ_ERROR, 2, BUZZER_SHORT_MS, BUZZER_GAP_MS);
}

void STM32BuzzerManager::beepUnknown() {
    startPattern(BUZZER_FREQ_ERROR, 1, BUZZER_LONG_MS, BUZZER_GAP_MS);
}

void STM32BuzzerManager::beepNetworkError() {
    startPattern(BUZZER_FREQ_ERROR, 3, BUZZER_SHORT_MS, BUZZER_GAP_MS);
}

#endif // ARDUINO_ARCH_STM32

