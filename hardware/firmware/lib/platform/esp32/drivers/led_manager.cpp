// ============================================================================
// led_manager.cpp — Non-blocking LED Blink Patterns
// ============================================================================

#include "led_manager.h"
#include "logger.h"
#include "pins.h"

void LedManager::init() {
    if (!LEDS_ENABLED) return;
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);
    allOff();
    LOG_INFO("LED Manager initialized");
}

void LedManager::update() {
    if (!LEDS_ENABLED || _pattern.state == PatternState::IDLE || _pattern.state == PatternState::DONE) {
        return;
    }

    unsigned long now = millis();

    switch (_pattern.state) {
        case PatternState::ON:
            if (now - _pattern.timer >= _pattern.onDuration) {
                digitalWrite(_pattern.pin, LOW);
                _pattern.currentBlink++;

                if (_pattern.currentBlink >= _pattern.totalBlinks) {
                    _pattern.state = PatternState::DONE;
                } else {
                    _pattern.state = PatternState::GAP;
                    _pattern.timer = now;
                }
            }
            break;

        case PatternState::GAP:
            if (now - _pattern.timer >= _pattern.gapDuration) {
                digitalWrite(_pattern.pin, HIGH);
                _pattern.state = PatternState::ON;
                _pattern.timer = now;
            }
            break;

        default:
            break;
    }
}

void LedManager::blinkSuccess() {
    if (!LEDS_ENABLED) return;
    LOG_DEBUG("LED: success pattern (1x green short)");
    allOff();
    startPattern(PIN_LED_GREEN, 1, LED_SHORT_BLINK_MS, LED_GAP_MS);
}

void LedManager::blinkDuplicate() {
    if (!LEDS_ENABLED) return;
    LOG_DEBUG("LED: duplicate pattern (2x red short)");
    allOff();
    startPattern(PIN_LED_RED, 2, LED_SHORT_BLINK_MS, LED_GAP_MS);
}

void LedManager::blinkFailure() {
    if (!LEDS_ENABLED) return;
    LOG_DEBUG("LED: failure pattern (1x red long)");
    allOff();
    startPattern(PIN_LED_RED, 1, LED_LONG_BLINK_MS, LED_GAP_MS);
}

void LedManager::greenOn()  { if (LEDS_ENABLED) digitalWrite(PIN_LED_GREEN, HIGH); }
void LedManager::greenOff() { if (LEDS_ENABLED) digitalWrite(PIN_LED_GREEN, LOW); }
void LedManager::redOn()    { if (LEDS_ENABLED) digitalWrite(PIN_LED_RED, HIGH); }
void LedManager::redOff()   { if (LEDS_ENABLED) digitalWrite(PIN_LED_RED, LOW); }

void LedManager::allOff() {
    if (!LEDS_ENABLED) return;
    digitalWrite(PIN_LED_GREEN, LOW);
    digitalWrite(PIN_LED_RED, LOW);
}

void LedManager::startPattern(int pin, uint8_t count, unsigned long onMs, unsigned long gapMs) {
    if (!LEDS_ENABLED) return;
    _pattern.pin          = pin;
    _pattern.totalBlinks  = count;
    _pattern.currentBlink = 0;
    _pattern.onDuration   = onMs;
    _pattern.gapDuration  = gapMs;
    _pattern.timer        = millis();
    _pattern.state        = PatternState::ON;

    digitalWrite(pin, HIGH);
}
