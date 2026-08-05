#pragma once
#include "IBuzzer.h"

class STM32BuzzerManager : public IBuzzer {
public:
    STM32BuzzerManager() {}
    ~STM32BuzzerManager() = default;

    void init() override {}
    void update() override {}
    void successBeep() override {}
    void errorBeep() override {}
    void duplicateBeep() override {}
    void bootBeep() override {}
};
