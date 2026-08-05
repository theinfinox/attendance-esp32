// ============================================================================
// buzzer_manager.cpp — Non-blocking Buzzer Tone Patterns (ESP32 LEDC PWM)
// ============================================================================

#include "buzzer_manager.h"
#include "logger.h"
#include "pins.h"

constexpr uint8_t BUZZER_LEDC_CHANNEL = 0;

static void playTone(int pin, unsigned int freq) {
    if (!BUZZER_ENABLED || freq == 0) return;
    ledcSetup(BUZZER_LEDC_CHANNEL, freq, 8);
    ledcAttachPin(pin, BUZZER_LEDC_CHANNEL);
    ledcWriteTone(BUZZER_LEDC_CHANNEL, freq);
}

static void stopTone(int pin) {
    if (!BUZZER_ENABLED) return;
    ledcWriteTone(BUZZER_LEDC_CHANNEL, 0);
    ledcDetachPin(pin);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void BuzzerManager::init() {
    if (!BUZZER_ENABLED) {
        LOG_INFO("Buzzer Manager initialized (DISABLED via constants.h)");
        return;
    }
    pinMode(PIN_BUZZER, OUTPUT);
    stopTone(PIN_BUZZER);
    silence();
    LOG_INFO("Buzzer Manager initialized");
}

void BuzzerManager::update() {
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

void BuzzerManager::beepSuccess() {
    if (!BUZZER_ENABLED) return;
    LOG_DEBUG("Buzzer: success (1x short high)");
    startPattern(BUZZER_FREQ_SUCCESS, 1, BUZZER_SHORT_MS, BUZZER_GAP_MS);
}

void BuzzerManager::beepDuplicate() {
    if (!BUZZER_ENABLED) return;
    LOG_DEBUG("Buzzer: duplicate (2x short)");
    startPattern(BUZZER_FREQ_SUCCESS, 2, BUZZER_SHORT_MS, BUZZER_GAP_MS);
}

void BuzzerManager::beepUnknown() {
    if (!BUZZER_ENABLED) return;
    LOG_DEBUG("Buzzer: unknown card (1x long low)");
    startPattern(BUZZER_FREQ_ERROR, 1, BUZZER_LONG_MS, BUZZER_GAP_MS);
}

void BuzzerManager::beepNetworkError() {
    if (!BUZZER_ENABLED) return;
    LOG_DEBUG("Buzzer: network error (3x short low)");
    startPattern(BUZZER_FREQ_ERROR, 3, BUZZER_SHORT_MS, BUZZER_GAP_MS);
}

void BuzzerManager::silence() {
    if (!BUZZER_ENABLED) return;
    stopTone(PIN_BUZZER);
    _pattern.state = ToneState::IDLE;
}

void BuzzerManager::startPattern(unsigned int freq, uint8_t count,
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
