// ============================================================================
// wifi_manager.cpp — WiFi Connection Manager
// ============================================================================

#include "wifi_manager.h"
#include "logger.h"
#include <time.h>

void WifiManager::init() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    // Lower TX power to prevent brownouts on weak USB power supplies
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    LOG_INFO("WiFi Manager initialized (STA mode, TX=8.5dBm)");
}

void WifiManager::connect() {
    _attempt++;
    _connecting = true;
    _connectStartMs = millis();
    LOG_INFO("WiFi connecting to '%s' (attempt %d)", CFG_WIFI_SSID, _attempt);
    WiFi.begin(CFG_WIFI_SSID, CFG_WIFI_PASSWORD);
}

void WifiManager::disconnect() {
    WiFi.disconnect();
    _connecting = false;
    LOG_INFO("WiFi disconnected");
}

void WifiManager::update() {
    bool nowConnected = (WiFi.status() == WL_CONNECTED);

    // Edge detection
    _justConnected    = (nowConnected && !_wasConnected);
    _justDisconnected = (!nowConnected && _wasConnected);

    if (_justConnected) {
        _connecting = false;
        _attempt = 0;
        LOG_INFO("WiFi connected! IP: %s  RSSI: %d dBm",
                 WiFi.localIP().toString().c_str(), WiFi.RSSI());
        
        // Sync time via NTP
        LOG_INFO("Syncing time with NTP (Offset: %ld sec)", CFG_TIMEZONE_OFFSET);
        configTime(CFG_TIMEZONE_OFFSET, CFG_DAYLIGHT_OFFSET, "pool.ntp.org", "time.nist.gov");
    }

    if (_justDisconnected) {
        LOG_WARN("WiFi connection lost");
    }

    // Auto-reconnect logic
    if (!nowConnected && !_connecting) {
        unsigned long now = millis();
        if (now - _lastReconnectMs >= WIFI_RECONNECT_INTERVAL_MS) {
            _lastReconnectMs = now;
            connect();
        }
    }

    // Timeout on current connection attempt
    if (_connecting && !nowConnected) {
        if (millis() - _connectStartMs >= WIFI_CONNECT_TIMEOUT_MS) {
            LOG_WARN("WiFi connect attempt %d timed out", _attempt);
            _connecting = false;
            // Will retry on next update cycle via auto-reconnect logic
        }
    }

    _wasConnected = nowConnected;
}

bool WifiManager::connected() const {
    return WiFi.status() == WL_CONNECTED;
}

int WifiManager::rssi() const {
    return connected() ? WiFi.RSSI() : 0;
}

String WifiManager::ip() const {
    return connected() ? WiFi.localIP().toString() : String("");
}

String WifiManager::mac() const {
    return WiFi.macAddress();
}
