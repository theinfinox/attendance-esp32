#if defined(ARDUINO_ARCH_STM32)
// ============================================================================
// led_manager.cpp — STM32 Non-blocking LED Blink Patterns
// ============================================================================

#include "led_manager.h"
#include "logger.h"
#include "pins.h"

void STM32LedManager::init() {
    if (!LEDS_ENABLED) return;
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);
    allOff();
    LOG_INFO("STM32 LED Manager initialized");
}

void STM32LedManager::update() {
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

void STM32LedManager::greenOn() {
    if (LEDS_ENABLED) digitalWrite(PIN_LED_GREEN, HIGH);
}

void STM32LedManager::greenOff() {
    if (LEDS_ENABLED) digitalWrite(PIN_LED_GREEN, LOW);
}

void STM32LedManager::redOn() {
    if (LEDS_ENABLED) digitalWrite(PIN_LED_RED, HIGH);
}

void STM32LedManager::redOff() {
    if (LEDS_ENABLED) digitalWrite(PIN_LED_RED, LOW);
}

void STM32LedManager::allOff() {
    greenOff();
    redOff();
    _pattern.state = PatternState::IDLE;
}

void STM32LedManager::startPattern(int pin, uint8_t count, unsigned long onMs, unsigned long gapMs) {
    if (!LEDS_ENABLED) return;
    allOff();
    _pattern.pin          = pin;
    _pattern.totalBlinks  = count;
    _pattern.currentBlink = 0;
    _pattern.onDuration   = onMs;
    _pattern.gapDuration  = gapMs;
    _pattern.timer        = millis();
    _pattern.state        = PatternState::ON;

    digitalWrite(pin, HIGH);
}

void STM32LedManager::blinkSuccess() {
    startPattern(PIN_LED_GREEN, 1, LED_SHORT_BLINK_MS, LED_GAP_MS);
}

void STM32LedManager::blinkDuplicate() {
    startPattern(PIN_LED_RED, 2, LED_SHORT_BLINK_MS, LED_GAP_MS);
}

void STM32LedManager::blinkFailure() {
    startPattern(PIN_LED_RED, 1, LED_LONG_BLINK_MS, LED_GAP_MS);
}

#endif // ARDUINO_ARCH_STM32

