#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
// ============================================================================
// display_manager.cpp — SSD1306 OLED Display Renderer (U8g2)
// ============================================================================

#include "display_manager.h"
#include "logger.h"
#include "pins.h"
#include "version.h"

// ---- Constructor -----------------------------------------------------------
DisplayManager::DisplayManager()
    : _display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, PIN_OLED_SCL, PIN_OLED_SDA)
{
}

// ---- Init ------------------------------------------------------------------
void DisplayManager::init() {
    _display.begin();
    _display.setFont(u8g2_font_6x10_tf);
    _display.clearBuffer();
    _display.sendBuffer();
    LOG_INFO("Display Manager initialized (SSD1306 128x64 I2C)");
}

// ---- Helpers ---------------------------------------------------------------

void DisplayManager::drawCentered(const char* text, int y, const uint8_t* font) {
    _display.setFont(font);
    int w = _display.getStrWidth(text);
    int x = (128 - w) / 2;
    if (x < 0) x = 0;
    _display.drawStr(x, y, text);
}

void DisplayManager::drawStatusScreen(const char* title, const char* subtitle,
                                       const char* iconChar) {
    _display.clearBuffer();

    // Icon area — large character centered at top
    drawCentered(iconChar, 28, u8g2_font_open_iconic_check_2x_t);

    // Title
    drawCentered(title, 46, u8g2_font_7x14B_tf);

    // Subtitle
    if (subtitle && subtitle[0] != '\0') {
        drawCentered(subtitle, 60, u8g2_font_5x8_tf);
    }

    _display.sendBuffer();
}

void DisplayManager::drawWifiIndicator(int rssi) {
    // Draw 4 bars in top-right corner based on signal strength
    int bars = 0;
    if (rssi > -50)      bars = 4;
    else if (rssi > -60) bars = 3;
    else if (rssi > -70) bars = 2;
    else if (rssi > -80) bars = 1;

    int baseX = 112;
    int baseY = 10;
    for (int i = 0; i < 4; i++) {
        int barHeight = 2 + i * 2;
        int x = baseX + i * 4;
        int y = baseY - barHeight;
        if (i < bars) {
            _display.drawBox(x, y, 3, barHeight);
        } else {
            _display.drawFrame(x, y, 3, barHeight);
        }
    }
}

void DisplayManager::drawProgressBar(int x, int y, int width, int height,
                                      uint8_t percent) {
    if (percent > 100) percent = 100;
    _display.drawFrame(x, y, width, height);
    int fillWidth = ((width - 2) * percent) / 100;
    if (fillWidth > 0) {
        _display.drawBox(x + 1, y + 1, fillWidth, height - 2);
    }
}

// ---- Screens ---------------------------------------------------------------

void DisplayManager::showBoot(uint8_t progressPercent) {
    _display.clearBuffer();

    // Project name
    drawCentered("IoT Attendance", 20, u8g2_font_7x14B_tf);

    // Version
    char verBuf[24];
    snprintf(verBuf, sizeof(verBuf), "v%s", FW_VERSION);
    drawCentered(verBuf, 34, u8g2_font_5x8_tf);

    // "Loading..."
    drawCentered("Loading...", 48, u8g2_font_6x10_tf);

    // Progress bar
    drawProgressBar(14, 52, 100, 8, progressPercent);

    _display.sendBuffer();
}

void DisplayManager::showInit() {
    _display.clearBuffer();
    drawCentered("Initializing", 24, u8g2_font_7x14B_tf);
    drawCentered("Hardware...", 40, u8g2_font_6x10_tf);
    _display.sendBuffer();
}

void DisplayManager::showWiFiConnecting(int attempt) {
    _display.clearBuffer();
    drawCentered("Connecting WiFi", 24, u8g2_font_7x14B_tf);

    // Animated dots based on attempt count
    String dots = "";
    int numDots = (attempt % 4);
    for (int i = 0; i < numDots; i++) dots += ".";
    // Pad to keep position stable
    while ((int)dots.length() < 3) dots += " ";

    char buf[32];
    snprintf(buf, sizeof(buf), "Please wait%s", dots.c_str());
    drawCentered(buf, 42, u8g2_font_6x10_tf);

    if (attempt > 0) {
        snprintf(buf, sizeof(buf), "Attempt %d", attempt);
        drawCentered(buf, 58, u8g2_font_5x8_tf);
    }

    _display.sendBuffer();
}

void DisplayManager::showBackendConnecting() {
    _display.clearBuffer();
    drawCentered("Connecting", 24, u8g2_font_7x14B_tf);
    drawCentered("Backend Server...", 42, u8g2_font_6x10_tf);
    _display.sendBuffer();
}

void DisplayManager::showReady(const String& ip, int rssi, const String& timeStr) {
    _display.clearBuffer();

    // WiFi indicator in top-right
    if (rssi != 0) {
        drawWifiIndicator(rssi);
    }
    
    // Time top-left
    if (timeStr.length() > 0) {
        _display.setFont(u8g2_font_5x8_tf);
        _display.drawStr(0, 8, timeStr.c_str());
    }

    // Main message
    drawCentered("READY", 28, u8g2_font_9x18B_tf);
    drawCentered("Tap your card", 44, u8g2_font_6x10_tf);

    // IP address at bottom
    if (ip.length() > 0) {
        char buf[20];
        snprintf(buf, sizeof(buf), "%s", ip.c_str());
        drawCentered(buf, 60, u8g2_font_5x8_tf);
    }

    _display.sendBuffer();
}

void DisplayManager::showCardDetected(const String& uid) {
    _display.clearBuffer();
    
    if (uid.length() > 0) {
        drawCentered("Card Detected", 24, u8g2_font_7x14B_tf);
        char buf[24];
        snprintf(buf, sizeof(buf), "UID: %s", uid.c_str());
        drawCentered(buf, 42, u8g2_font_6x10_tf);
    } else {
        drawCentered("Card Detected", 32, u8g2_font_7x14B_tf);
    }

    drawCentered("Processing...", 58, u8g2_font_5x8_tf);
    _display.sendBuffer();
}

void DisplayManager::showSuccess(const String& name) {
    _display.clearBuffer();

    // Checkmark icon (U+2713 from open_iconic)
    drawCentered("\x40", 24, u8g2_font_open_iconic_check_2x_t);  // checkmark glyph

    if (name.length() > 0) {
        drawCentered(name.c_str(), 42, u8g2_font_7x14B_tf);
        drawCentered("Attendance Marked", 58, u8g2_font_5x8_tf);
    } else {
        drawCentered("Attendance", 42, u8g2_font_7x14B_tf);
        drawCentered("Marked!", 56, u8g2_font_7x14B_tf);
    }

    _display.sendBuffer();
}

void DisplayManager::showDuplicate() {
    _display.clearBuffer();
    drawCentered("Already", 20, u8g2_font_7x14B_tf);
    drawCentered("Marked", 36, u8g2_font_7x14B_tf);
    drawCentered("Duplicate scan", 54, u8g2_font_5x8_tf);
    _display.sendBuffer();
}

void DisplayManager::showUnknownCard(const String& uid) {
    _display.clearBuffer();
    drawCentered("Unknown Card", 20, u8g2_font_7x14B_tf);
    
    if (uid.length() > 0) {
        char buf[24];
        snprintf(buf, sizeof(buf), "UID: %s", uid.c_str());
        drawCentered(buf, 38, u8g2_font_6x10_tf);
        drawCentered("Not registered", 54, u8g2_font_5x8_tf);
    } else {
        drawCentered("Not registered", 38, u8g2_font_6x10_tf);
        drawCentered("Contact admin", 54, u8g2_font_5x8_tf);
    }
    _display.sendBuffer();
}

void DisplayManager::showNetworkError() {
    _display.clearBuffer();
    drawCentered("Network Error", 24, u8g2_font_7x14B_tf);
    drawCentered("WiFi disconnected", 42, u8g2_font_6x10_tf);
    drawCentered("Reconnecting...", 58, u8g2_font_5x8_tf);
    _display.sendBuffer();
}

void DisplayManager::showBackendError() {
    _display.clearBuffer();
    drawCentered("Server Error", 24, u8g2_font_7x14B_tf);
    drawCentered("Backend unreachable", 42, u8g2_font_6x10_tf);
    drawCentered("Retrying...", 58, u8g2_font_5x8_tf);
    _display.sendBuffer();
}

void DisplayManager::showAuthError() {
    _display.clearBuffer();
    drawCentered("Auth Error", 24, u8g2_font_7x14B_tf);
    drawCentered("Device not", 42, u8g2_font_6x10_tf);
    drawCentered("authorized", 56, u8g2_font_5x8_tf);
    _display.sendBuffer();
}

void DisplayManager::showShutdown(const String& reason) {
    _display.clearBuffer();
    drawCentered("ERROR", 20, u8g2_font_7x14B_tf);

    char buf[32];
    snprintf(buf, sizeof(buf), "%s", reason.c_str());
    drawCentered(buf, 40, u8g2_font_6x10_tf);

    drawCentered("Restart device", 58, u8g2_font_5x8_tf);
    _display.sendBuffer();
}

void DisplayManager::showFailed() {
    drawStatusScreen("Failed", "Operation Error", "!");
}

#endif // ARDUINO_ARCH_ESP32
