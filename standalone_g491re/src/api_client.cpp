// ============================================================================
// api_client.cpp — Backend API Client (Mocked Implementation)
// Cycles through SUCCESS -> DUPLICATE -> UNKNOWN_CARD to demo all UI states.
// ============================================================================

#include "api_client.h"
#include "logger.h"
#include <ArduinoJson.h>

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
            unsigned long nowEpoch = _repo->getEpoch();
            String eventType = "Check-in";
            uint16_t session = 1;

            bool stored = _repo->recordPunch(uid, name, nowEpoch, eventType, session);
            if (!stored) {
                resp.result = AttendanceResult::BACKEND_ERROR;
                resp.message = "Storage Full! Please export logs.";
                LOG_ERROR("API [STANDALONE]: STORAGE FULL for %s (UID=%s)", name.c_str(), uid.c_str());
            } else {
                resp.result = AttendanceResult::OK;
                resp.message = eventType + " marked successfully";
                resp.name = name;
                resp.eventType = eventType;
                resp.session = session;

                // Emit clean JSON message with type: "log" for PWA live-stream
                JsonDocument logDoc;
                logDoc["type"] = "log";
                JsonObject data = logDoc["data"].to<JsonObject>();
                data["uid"] = uid;
                data["name"] = name;
                data["timestamp"] = nowEpoch;
                data["event"] = eventType;
                data["status"] = (eventType == "Check-out") ? "OUT" : "IN";
                data["session"] = session;
                String logJson;
                serializeJson(logDoc, logJson);
                Serial.println(logJson);

                LOG_INFO("API [STANDALONE]: SUCCESS for %s (UID=%s) [%s #%u]",
                         name.c_str(), uid.c_str(), eventType.c_str(), session);
            }
        }
        return resp;
    }

    if (_mocked) {
        return mockSubmitAttendance(uid);
    }

    AttendanceResponse resp;
    resp.result  = AttendanceResult::BACKEND_ERROR;
    resp.message = "Real API not implemented";
    return resp;
}

bool ApiClient::heartbeat() {
    if (CFG_STANDALONE_MODE) return true;
    if (_mocked) return mockHeartbeat();
    return false;
}

bool ApiClient::ping() {
    if (CFG_STANDALONE_MODE) return true;
    if (_mocked) return mockPing();
    return false;
}

// ---- Mock implementations --------------------------------------------------

AttendanceResponse ApiClient::mockSubmitAttendance(const String& uid) {
    AttendanceResponse resp;

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
