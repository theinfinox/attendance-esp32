#pragma once
#include <Arduino.h>

class IBuzzer {
public:
    virtual ~IBuzzer() = default;
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void beepSuccess() = 0;
    virtual void beepDuplicate() = 0;
    virtual void beepUnknown() = 0;
    virtual void beepNetworkError() = 0;
    virtual void beepError() = 0;
};
