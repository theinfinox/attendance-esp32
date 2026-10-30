#pragma once
// ============================================================================
// display_manager.h — STM32 SSD1306 OLED Display Renderer (U8g2)
// ============================================================================

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "constants.h"
#include "IDisplay.h"

class STM32DisplayManager : public IDisplay {
public:
    STM32DisplayManager();
    void init() override;

    // --- Screen rendering functions ---
    void showBoot(uint8_t progressPercent) override;
    void showInit() override;
    void showWiFiConnecting(int attempt = 0) override;
    void showBackendConnecting() override;
    void showReady(const String& ip = "", int rssi = 0, const String& timeStr = "") override;
    void showCardDetected(const String& uid) override;
    void showSuccess(const String& name = "") override;
    void showCheckIn(const String& name = "", const String& timeStr = "") override;
    void showCheckOut(const String& name = "", const String& timeStr = "") override;
    void showDuplicate() override;
    void showUnknownCard(const String& uid = "") override;
    void showNetworkError() override;
    void showBackendError() override;
    void showAuthError() override;
    void showFailed() override;
    void showShutdown(const String& reason = "Fatal Error") override;

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _display;

    void drawCentered(const char* text, int y, const uint8_t* font);
    void drawStatusScreen(const char* title, const char* subtitle, const char* iconChar);
    void drawProgressBar(int x, int y, int width, int height, uint8_t percent);
};
