#pragma once

class ILed {
public:
    virtual ~ILed() = default;
    virtual void init() = 0;
    virtual void update() = 0;
    virtual void greenOn() = 0;
    virtual void greenOff() = 0;
    virtual void redOn() = 0;
    virtual void redOff() = 0;
    virtual void allOff() = 0;

    virtual void blinkSuccess() = 0;
    virtual void blinkDuplicate() = 0;
    virtual void blinkFailure() = 0;
};
