import os

os.makedirs('lib/platform/stm32/config', exist_ok=True)
os.makedirs('lib/platform/stm32/drivers', exist_ok=True)

with open('lib/platform/stm32/config/pins.h', 'w') as f:
    f.write('#pragma once\n// Placeholder for STM32 Pins\n')

with open('lib/platform/stm32/drivers/display_manager.h', 'w') as f:
    f.write('''#pragma once
#include "IDisplay.h"

class STM32DisplayManager : public IDisplay {
public:
    STM32DisplayManager() {}
    ~STM32DisplayManager() = default;

    void init() override {}
    void showBoot(uint8_t progressPercent) override {}
    void showInit() override {}
    void showWiFiConnecting(int attempt = 0) override {}
    void showBackendConnecting() override {}
    void showReady(const String& ip = "", int rssi = 0, const String& timeStr = "") override {}
    void showCardDetected(const String& uid) override {}
    void showSuccess(const String& name = "") override {}
    void showDuplicate() override {}
    void showUnknownCard(const String& uid = "") override {}
    void showNetworkError() override {}
    void showBackendError() override {}
    void showAuthError() override {}
    void showFailed() override {}
    void showShutdown(const String& reason = "Fatal Error") override {}
};
''')

with open('lib/platform/stm32/drivers/rfid_manager.h', 'w') as f:
    f.write('''#pragma once
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
''')

with open('lib/platform/stm32/drivers/led_manager.h', 'w') as f:
    f.write('''#pragma once
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
''')

with open('lib/platform/stm32/drivers/buzzer_manager.h', 'w') as f:
    f.write('''#pragma once
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
''')
