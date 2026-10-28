#if defined(ARDUINO_ARCH_STM32)
// ============================================================================
// display_manager.cpp — STM32 SSD1306 OLED Display Renderer (U8g2)
// ============================================================================

#include "display_manager.h"
#include "logger.h"
#include "pins.h"
#include "version.h"

STM32DisplayManager::STM32DisplayManager()
    : _display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, PIN_OLED_SCL, PIN_OLED_SDA)
{
}

void STM32DisplayManager::init() {
    _display.begin();
    _display.setFont(u8g2_font_6x10_tf);
    _display.clearBuffer();
    _display.sendBuffer();
    LOG_INFO("STM32 Display Manager initialized (SSD1306 128x64 I2C)");
}

void STM32DisplayManager::drawCentered(const char* text, int y, const uint8_t* font) {
    _display.setFont(font);
    int w = _display.getStrWidth(text);
    int x = (128 - w) / 2;
    if (x < 0) x = 0;
    _display.drawStr(x, y, text);
}

void STM32DisplayManager::drawStatusScreen(const char* title, const char* subtitle,
                                            const char* iconChar) {
    _display.clearBuffer();
    drawCentered(iconChar, 28, u8g2_font_open_iconic_check_2x_t);
    drawCentered(title, 46, u8g2_font_7x14B_tf);
    if (subtitle && subtitle[0] != '\0') {
        drawCentered(subtitle, 60, u8g2_font_5x8_tf);
    }
    _display.sendBuffer();
}

void STM32DisplayManager::drawProgressBar(int x, int y, int width, int height, uint8_t percent) {
    _display.drawFrame(x, y, width, height);
    int fillW = ((width - 4) * percent) / 100;
    if (fillW > 0) {
        _display.drawBox(x + 2, y + 2, fillW, height - 4);
    }
}

void STM32DisplayManager::showBoot(uint8_t progressPercent) {
    _display.clearBuffer();
    drawCentered(FW_PROJECT_NAME, 20, u8g2_font_7x14B_tf);
    char verBuf[24];
    snprintf(verBuf, sizeof(verBuf), "v%s (STM32)", FW_VERSION);
    drawCentered(verBuf, 34, u8g2_font_5x8_tf);
    drawProgressBar(14, 42, 100, 10, progressPercent);
    char pctBuf[8];
    snprintf(pctBuf, sizeof(pctBuf), "%d%%", progressPercent);
    drawCentered(pctBuf, 62, u8g2_font_5x8_tf);
    _display.sendBuffer();
}

void STM32DisplayManager::showInit() {
    _display.clearBuffer();
    drawCentered("Starting...", 30, u8g2_font_7x14B_tf);
    drawCentered("Initializing hardware", 48, u8g2_font_5x8_tf);
    _display.sendBuffer();
}

void STM32DisplayManager::showWiFiConnecting(int attempt) {
    _display.clearBuffer();
    drawCentered("Offline Mode", 32, u8g2_font_7x14B_tf);
    drawCentered("Serial Connection Active", 48, u8g2_font_5x8_tf);
    _display.sendBuffer();
}

void STM32DisplayManager::showBackendConnecting() {
    _display.clearBuffer();
    drawCentered("Connecting...", 32, u8g2_font_7x14B_tf);
    drawCentered("Checking Serial CLI", 48, u8g2_font_5x8_tf);
    _display.sendBuffer();
}

void STM32DisplayManager::showReady(const String& ip, int rssi, const String& timeStr) {
    _display.clearBuffer();
    _display.drawHLine(0, 12, 128);

    _display.setFont(u8g2_font_5x8_tf);
    _display.drawStr(0, 9, "STM32 Ready");

    if (timeStr.length() > 0) {
        int w = _display.getStrWidth(timeStr.c_str());
        _display.drawStr(128 - w, 9, timeStr.c_str());
    }

    drawCentered("TAP ID CARD", 34, u8g2_font_7x14B_tf);
    drawCentered("USB Serial: 115200", 50, u8g2_font_5x8_tf);

    String statusLine = ip.length() > 0 ? ip : "Serial Mode";
    drawCentered(statusLine.c_str(), 62, u8g2_font_5x8_tf);

    _display.sendBuffer();
}

void STM32DisplayManager::showCardDetected(const String& uid) {
    _display.clearBuffer();
    drawCentered("CARD DETECTED", 24, u8g2_font_7x14B_tf);
    drawCentered("Processing...", 42, u8g2_font_6x10_tf);
    if (uid.length() > 0) {
        drawCentered(uid.c_str(), 58, u8g2_font_5x8_tf);
    }
    _display.sendBuffer();
}

void STM32DisplayManager::showSuccess(const String& name) {
    _display.clearBuffer();
    drawCentered("C", 24, u8g2_font_open_iconic_check_2x_t);
    drawCentered("WELCOME!", 40, u8g2_font_7x14B_tf);
    String displayName = name.length() > 0 ? name : "Attendance Marked";
    drawCentered(displayName.c_str(), 54, u8g2_font_6x10_tf);
    drawCentered("HAVE A GREAT DAY", 63, u8g2_font_4x6_tf);
    _display.sendBuffer();
}

void STM32DisplayManager::showDuplicate() {
    _display.clearBuffer();
    drawCentered("!", 24, u8g2_font_open_iconic_check_2x_t);
    drawCentered("ALREADY SCANNED", 42, u8g2_font_7x14B_tf);
    drawCentered("Please wait before scanning again", 58, u8g2_font_4x6_tf);
    _display.sendBuffer();
}

void STM32DisplayManager::showUnknownCard(const String& uid) {
    _display.clearBuffer();
    drawCentered("X", 24, u8g2_font_open_iconic_check_2x_t);
    drawCentered("CARD NOT FOUND", 40, u8g2_font_7x14B_tf);
    if (uid.length() > 0) {
        drawCentered(uid.c_str(), 54, u8g2_font_5x8_tf);
    }
    drawCentered("Register card via PWA", 63, u8g2_font_4x6_tf);
    _display.sendBuffer();
}

void STM32DisplayManager::showNetworkError() {
    drawStatusScreen("COMM ERROR", "Check USB Connection", "E");
}

void STM32DisplayManager::showBackendError() {
    drawStatusScreen("STORAGE ERROR", "Flash/SD Error", "E");
}

void STM32DisplayManager::showAuthError() {
    drawStatusScreen("AUTH ERROR", "Access Denied", "X");
}

void STM32DisplayManager::showFailed() {
    drawStatusScreen("SCAN FAILED", "Please try again", "X");
}

void STM32DisplayManager::showShutdown(const String& reason) {
    drawStatusScreen("SYSTEM HALTED", reason.c_str(), "X");
}

#endif // ARDUINO_ARCH_STM32

