// ============================================================================
// state_machine.cpp — Firmware State Machine
// ============================================================================

#include "state_machine.h"
#include "logger.h"

void StateMachine::init() {
    _currentState  = SystemState::BOOT;
    _previousState = SystemState::BOOT;
    _stateEntryMs  = millis();
    _stateChanged  = true;
    LOG_INFO("State Machine initialized → BOOT");
}

bool StateMachine::transition(SystemState newState) {
    if (newState == _currentState) {
        return false;  // Already in this state
    }

    if (!isValidTransition(_currentState, newState)) {
        LOG_WARN("Invalid state transition: %s → %s",
                 stateToString(_currentState), stateToString(newState));
        return false;
    }

    LOG_INFO("State: %s → %s", stateToString(_currentState), stateToString(newState));

    _previousState = _currentState;
    _currentState  = newState;
    _stateEntryMs  = millis();
    _stateChanged  = true;

    return true;
}

unsigned long StateMachine::timeInState() const {
    return millis() - _stateEntryMs;
}

bool StateMachine::shouldAutoReturn() const {
    // Result states auto-return to READY after display duration
    switch (_currentState) {
        case SystemState::SUCCESS:
        case SystemState::FAILED:
            return timeInState() >= DISPLAY_RESULT_DURATION_MS;
        default:
            return false;
    }
}

bool StateMachine::isValidTransition(SystemState from, SystemState to) const {
    // Define valid transitions
    switch (from) {
        case SystemState::BOOT:
            return to == SystemState::INIT;

        case SystemState::INIT:
            return to == SystemState::WIFI_CONNECTING;

        case SystemState::WIFI_CONNECTING:
            return to == SystemState::BACKEND_CONNECTING
                || to == SystemState::NETWORK_ERROR;

        case SystemState::BACKEND_CONNECTING:
            return to == SystemState::READY
                || to == SystemState::NETWORK_ERROR
                || to == SystemState::FAILED;

        case SystemState::READY:
            return to == SystemState::CARD_DETECTED
                || to == SystemState::NETWORK_ERROR;

        case SystemState::CARD_DETECTED:
            return to == SystemState::SENDING
                || to == SystemState::READY
                || to == SystemState::NETWORK_ERROR;

        case SystemState::SENDING:
            return to == SystemState::SUCCESS
                || to == SystemState::FAILED
                || to == SystemState::NETWORK_ERROR;

        case SystemState::SUCCESS:
            return to == SystemState::READY
                || to == SystemState::CARD_DETECTED
                || to == SystemState::NETWORK_ERROR;

        case SystemState::FAILED:
            return to == SystemState::READY
                || to == SystemState::CARD_DETECTED
                || to == SystemState::NETWORK_ERROR;

        case SystemState::NETWORK_ERROR:
            return to == SystemState::WIFI_CONNECTING
                || to == SystemState::READY
                || to == SystemState::BACKEND_CONNECTING;

        default:
            return false;
    }
}
