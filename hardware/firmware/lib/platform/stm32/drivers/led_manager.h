#pragma once
#include "ILed.h"

class STM32LedManager : public ILed {
public:
    STM32LedManager() {}
    ~STM32LedManager() = default;

    void init() override {}
    void update() override {}
    void greenOn() override {}
    void greenOff() override {}
    void redOn() override {}
    void redOff() override {}
    void allOff() override {}
    void blinkSuccess() override {}
    void blinkDuplicate() override {}
    void blinkFailure() override {}
};
