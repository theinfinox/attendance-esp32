#pragma once
#include <Arduino.h>

class IDisplay {
public:
    virtual ~IDisplay() = default;
    virtual void init() = 0;
    virtual void showBoot(uint8_t progressPercent) = 0;
    virtual void showInit() = 0;
    virtual void showWiFiConnecting(int attempt = 0) = 0;
    virtual void showBackendConnecting() = 0;
    virtual void showReady(const String& ip = "", int rssi = 0, const String& timeStr = "") = 0;
    virtual void showCardDetected(const String& uid) = 0;
    virtual void showSuccess(const String& name = "") = 0;
    virtual void showCheckIn(const String& name = "", const String& timeStr = "") { showSuccess(name); }
    virtual void showCheckOut(const String& name = "", const String& timeStr = "") { showSuccess(name); }
    virtual void showDuplicate() = 0;
    virtual void showUnknownCard(const String& uid = "") = 0;
    virtual void showNetworkError() = 0;
    virtual void showBackendError() = 0;
    virtual void showAuthError() = 0;
    virtual void showFailed() = 0;
    virtual void showShutdown(const String& reason = "Fatal Error") = 0;
};
