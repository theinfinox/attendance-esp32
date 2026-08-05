#pragma once
#include "IRFID.h"

class STM32RfidManager : public IRFID {
public:
    STM32RfidManager() {}
    ~STM32RfidManager() = default;

    void init() override {}
    void update() override {}
    bool ready() const override { return true; }
    bool cardDetected() override { return false; }
    String lastUID() override { return ""; }
};
