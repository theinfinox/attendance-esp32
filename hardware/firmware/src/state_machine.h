#pragma once
// ============================================================================
// state_machine.h — Firmware State Machine
// Clean state transitions with logging and time-in-state tracking.
// ============================================================================

#include <Arduino.h>
#include "types.h"
#include "constants.h"

class StateMachine {
public:
    void init();

    // Transition to a new state (validates and logs)
    bool transition(SystemState newState);

    // Current state
    SystemState currentState() const { return _currentState; }
    SystemState previousState() const { return _previousState; }

    // Time spent in current state (ms)
    unsigned long timeInState() const;

    // Did the state just change this cycle?
    bool stateChanged() const { return _stateChanged; }

    // Reset the stateChanged flag (call at end of each loop)
    void clearStateChanged() { _stateChanged = false; }

    // Should the current result screen auto-return to READY?
    bool shouldAutoReturn() const;

private:
    SystemState   _currentState  = SystemState::BOOT;
    SystemState   _previousState = SystemState::BOOT;
    unsigned long _stateEntryMs  = 0;
    bool          _stateChanged  = false;

    // Validate whether a transition is allowed
    bool isValidTransition(SystemState from, SystemState to) const;
};
