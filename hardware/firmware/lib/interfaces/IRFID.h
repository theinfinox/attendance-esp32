#pragma once
#include <Arduino.h>

class IRFID {
public:
    virtual ~IRFID() = default;
    virtual void init() = 0;
    virtual void update() = 0;
    virtual bool ready() const = 0;
    virtual bool cardDetected() = 0;
    virtual String lastUID() = 0;
};
