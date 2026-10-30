#pragma once
// ============================================================================
// config.h — Application Configuration
// WiFi, server, device identity. Replace with AP provisioning later.
// ============================================================================

// --- Global ---
constexpr bool CFG_DEBUG_MODE       = true; // True: Mock API, verbose logs, Serial simulation. False: Strict production.
constexpr bool CFG_STANDALONE_MODE  = true; // True: Embedded backend via LittleFS/WebServer. False: Cloud backend.
// --- UI Settings ---
constexpr bool CFG_SHOW_UID_ON_DETECT = true;  // If true, shows the UID on the 'Card Detected' processing screen

// --- Time / NTP ---
constexpr long CFG_TIMEZONE_OFFSET  = 19800; // GMT+5:30 in seconds (India)
constexpr long CFG_DAYLIGHT_OFFSET  = 0;

// --- WiFi ---
constexpr const char* CFG_WIFI_SSID     = "pi";
constexpr const char* CFG_WIFI_PASSWORD  = "raspberry";

// --- Backend API ---
constexpr const char* CFG_SERVER_URL     = "https://attendance.govindsr.me";
constexpr const char* CFG_API_VERSION    = "v1";
constexpr const char* CFG_API_BASE_PATH  = "/api/v1";

// --- Device Identity ---
constexpr const char* CFG_DEVICE_ID      = "ESP32-ATT-001";
