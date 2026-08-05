#pragma once

#include <Arduino.h>

// --- System State Machine ---
enum class SystemState : uint8_t {
    BOOT,
    INIT,
    WIFI_CONNECTING,
    BACKEND_CONNECTING,
    READY,
    CARD_DETECTED,
    SENDING,
    SUCCESS,
    FAILED,
    NETWORK_ERROR
};

// --- Attendance API Result ---
enum class AttendanceResult : uint8_t {
    OK,              // Attendance marked successfully
    DUPLICATE,       // Already marked for this session
    UNKNOWN_CARD,    // Card UID not registered
    NETWORK_ERROR,   // Could not reach backend
    BACKEND_ERROR,   // Backend returned server error
    AUTH_ERROR       // Device not authorized
};

// --- Shared Models ---
struct Student {
    String uid;
    String name;
};

struct AttendanceLog {
    String uid;
    String name;
    unsigned long timestamp;
};

struct AttendanceResponse {
    AttendanceResult result   = AttendanceResult::NETWORK_ERROR;
    String           message  = "";
    String           name     = "";  // Student name (if resolved)
};

// --- Utilities ---
inline const char* stateToString(SystemState state) {
    switch (state) {
        case SystemState::BOOT:               return "BOOT";
        case SystemState::INIT:               return "INIT";
        case SystemState::WIFI_CONNECTING:    return "WIFI_CONNECTING";
        case SystemState::BACKEND_CONNECTING: return "BACKEND_CONNECTING";
        case SystemState::READY:              return "READY";
        case SystemState::CARD_DETECTED:      return "CARD_DETECTED";
        case SystemState::SENDING:            return "SENDING";
        case SystemState::SUCCESS:            return "SUCCESS";
        case SystemState::FAILED:             return "FAILED";
        case SystemState::NETWORK_ERROR:      return "NETWORK_ERROR";
        default:                              return "UNKNOWN";
    }
}

inline const char* resultToString(AttendanceResult result) {
    switch (result) {
        case AttendanceResult::OK:            return "OK";
        case AttendanceResult::DUPLICATE:     return "DUPLICATE";
        case AttendanceResult::UNKNOWN_CARD:  return "UNKNOWN_CARD";
        case AttendanceResult::NETWORK_ERROR: return "NETWORK_ERROR";
        case AttendanceResult::BACKEND_ERROR: return "BACKEND_ERROR";
        case AttendanceResult::AUTH_ERROR:    return "AUTH_ERROR";
        default:                              return "UNKNOWN";
    }
}
