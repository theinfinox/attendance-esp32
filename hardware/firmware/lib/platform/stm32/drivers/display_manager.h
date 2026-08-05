#pragma once
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
