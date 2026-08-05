#pragma once
// ============================================================================
// display_manager.h — SSD1306 OLED Display Renderer (U8g2)
// Single responsibility: rendering screens. Never reads sensors or network.
// ============================================================================

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "constants.h"
#include "IDisplay.h"

class DisplayManager : public IDisplay {
public:
    void init() override;

    // --- Screen rendering functions ---
    void showBoot(uint8_t progressPercent);
    void showInit() override;
    void showWiFiConnecting(int attempt = 0) override;
    void showBackendConnecting() override;
    void showReady(const String& ip = "", int rssi = 0, const String& timeStr = "") override;
    void showCardDetected(const String& uid) override;
    void showSuccess(const String& name = "") override;
    void showDuplicate() override;
    void showUnknownCard(const String& uid = "") override;
    void showNetworkError() override;
    void showBackendError();
    void showAuthError();
    void showFailed() override;
    void showShutdown(const String& reason = "Fatal Error");

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _display;

    // Helper: draw centered text at given y position
    void drawCentered(const char* text, int y, const uint8_t* font);

    // Helper: draw a standard status screen with icon area, title, and subtitle
    void drawStatusScreen(const char* title, const char* subtitle,
                          const char* iconChar);

    // Helper: draw WiFi signal bars based on RSSI
    void drawWifiIndicator(int rssi);

    // Helper: draw a progress bar
    void drawProgressBar(int x, int y, int width, int height, uint8_t percent);

public:
    // Constructor — sets up U8g2 instance but doesn't start I2C
    DisplayManager();
};
