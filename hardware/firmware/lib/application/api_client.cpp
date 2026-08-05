// ============================================================================
// api_client.cpp — Backend API Client (Mocked Implementation)
// Cycles through SUCCESS → DUPLICATE → UNKNOWN_CARD to demo all UI states.
// ============================================================================

#include "api_client.h"
#include "logger.h"

void ApiClient::init(IAttendanceRepository* repo) {
    _repo = repo;
    _mocked = CFG_DEBUG_MODE;
    _mockCounter = 0;
    
    if (CFG_STANDALONE_MODE) {
        LOG_INFO("API Client initialized (STANDALONE embedded mode)");
    } else {
        LOG_INFO("API Client initialized (%s mode)", _mocked ? "MOCKED" : "CLOUD");
        LOG_INFO("  Server URL: %s%s", CFG_SERVER_URL, CFG_API_BASE_PATH);
        LOG_INFO("  Device ID:  %s", CFG_DEVICE_ID);
    }
}

// ---- Public interface (delegates to mock or real) --------------------------

AttendanceResponse ApiClient::submitAttendance(const String& uid) {
    LOG_INFO("API: submitAttendance(uid=%s)", uid.c_str());

    if (CFG_STANDALONE_MODE) {
        AttendanceResponse resp;
        String name = _repo->getStudentName(uid);
        
        if (name.isEmpty()) {
            resp.result = AttendanceResult::UNKNOWN_CARD;
            resp.message = "Card not registered in system";
            LOG_INFO("API [STANDALONE]: UNKNOWN_CARD for UID=%s", uid.c_str());
        } else {
            // Get timestamp from time()
            time_t now;
            time(&now);
            
            _repo->markAttendance(uid, name, (unsigned long)now);
            
            resp.result = AttendanceResult::OK;
            resp.message = "Attendance marked successfully";
            resp.name = name;
            LOG_INFO("API [STANDALONE]: SUCCESS for %s (UID=%s)", name.c_str(), uid.c_str());
        }
        return resp;
    }

    if (_mocked) {
        return mockSubmitAttendance(uid);
    }

    // TODO: Real HTTPS implementation
    // HTTPClient http;
    // http.begin(String(CFG_SERVER_URL) + CFG_API_BASE_PATH + "/attendance");
    // http.addHeader("Content-Type", "application/json");
    // http.addHeader("X-Device-ID", CFG_DEVICE_ID);
    // ...
    AttendanceResponse resp;
    resp.result  = AttendanceResult::BACKEND_ERROR;
    resp.message = "Real API not implemented";
    return resp;
}

bool ApiClient::heartbeat() {
    if (CFG_STANDALONE_MODE) return true;
    if (_mocked) return mockHeartbeat();

    // TODO: Real implementation
    return false;
}

bool ApiClient::ping() {
    if (CFG_STANDALONE_MODE) return true;
    if (_mocked) return mockPing();

    // TODO: Real implementation
    return false;
}

// ---- Mock implementations --------------------------------------------------

AttendanceResponse ApiClient::mockSubmitAttendance(const String& uid) {
    AttendanceResponse resp;

    // Cycle: SUCCESS → DUPLICATE → UNKNOWN_CARD → SUCCESS → ...
    switch (_mockCounter % 3) {
        case 0:
            resp.result  = AttendanceResult::OK;
            resp.message = "Attendance marked successfully";
            resp.name    = "John Doe";
            LOG_INFO("API [MOCK]: SUCCESS for UID=%s", uid.c_str());
            break;
        case 1:
            resp.result  = AttendanceResult::DUPLICATE;
            resp.message = "Already marked for this session";
            resp.name    = "John Doe";
            LOG_INFO("API [MOCK]: DUPLICATE for UID=%s", uid.c_str());
            break;
        case 2:
            resp.result  = AttendanceResult::UNKNOWN_CARD;
            resp.message = "Card not registered in system";
            resp.name    = "";
            LOG_INFO("API [MOCK]: UNKNOWN_CARD for UID=%s", uid.c_str());
            break;
    }

    _mockCounter++;

    // Simulate network latency
    delay(200);

    return resp;
}

bool ApiClient::mockHeartbeat() {
    LOG_DEBUG("API [MOCK]: heartbeat OK");
    return true;
}

bool ApiClient::mockPing() {
    LOG_DEBUG("API [MOCK]: ping OK");
    return true;
}
