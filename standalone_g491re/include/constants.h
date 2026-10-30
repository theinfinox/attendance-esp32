#pragma once
// ============================================================================
// constants.h — Timing, Retry, and Threshold Constants
// ============================================================================

// --- WiFi ---
constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS   = 15000;
constexpr unsigned long WIFI_RECONNECT_INTERVAL_MS = 5000;

// --- Backend ---
constexpr unsigned long API_REQUEST_TIMEOUT_MS     = 5000;
constexpr int           API_MAX_RETRIES            = 3;
constexpr unsigned long HEARTBEAT_INTERVAL_MS      = 30000;

// --- RFID ---
constexpr unsigned long RFID_DEBOUNCE_MS           = 2000;  // Prevent duplicate reads
constexpr unsigned long RFID_POLL_INTERVAL_MS      = 200;   // How often to check for card

// --- Display ---
constexpr unsigned long DISPLAY_RESULT_DURATION_MS = 3000;  // Show success/error then return to Ready
constexpr unsigned long BOOT_ANIMATION_STEP_MS     = 100;   // Boot progress bar step

// --- LED ---
constexpr bool          LEDS_ENABLED               = false; // Disabled (no physical LEDs connected currently)
constexpr unsigned long LED_SHORT_BLINK_MS         = 150;
constexpr unsigned long LED_LONG_BLINK_MS          = 800;
constexpr unsigned long LED_GAP_MS                 = 150;

// --- Buzzer ---
constexpr bool          BUZZER_ENABLED             = false; // Disabled (no physical buzzer connected currently)
constexpr unsigned int  BUZZER_FREQ_SUCCESS        = 2000;  // Hz
constexpr unsigned int  BUZZER_FREQ_ERROR           = 500;
constexpr unsigned long BUZZER_SHORT_MS            = 100;
constexpr unsigned long BUZZER_LONG_MS             = 500;
constexpr unsigned long BUZZER_GAP_MS              = 100;

// --- General ---
constexpr unsigned long SERIAL_BAUD_RATE           = 115200;
