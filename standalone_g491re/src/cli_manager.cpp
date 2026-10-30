// ============================================================================
// cli_manager.cpp — Serial Command Line Interface
// ============================================================================

#include "cli_manager.h"
#include "logger.h"
#include "STM32FlashRepository.h"
#include "IRFID.h"
#include <time.h>
#include <ctype.h>

void CliManager::init(IAttendanceRepository* repo, IRFID* rfid) {
    _repo = repo;
    _rfid = rfid;
    _inputBuffer.reserve(64);
    LOG_INFO("CLI Manager initialized. Type HELP for commands.");
}

String CliManager::update() {
    String unhandled = "";
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (_inputBuffer.length() > 0) {
                if (!processCommand(_inputBuffer)) {
                    if (unhandled.length() == 0) unhandled = _inputBuffer;
                }
                _inputBuffer = "";
            }
        } else {
            _inputBuffer += c;
            // Prevent buffer overflow from garbage data
            if (_inputBuffer.length() > 100) {
                _inputBuffer = "";
            }
        }
    }
    return unhandled;
}

bool CliManager::processCommand(const String& cmd) {
    String upperCmd = cmd;
    upperCmd.trim();

    String cmdName = upperCmd;
    int spaceIdx = upperCmd.indexOf(' ');
    if (spaceIdx != -1) {
        cmdName = upperCmd.substring(0, spaceIdx);
    }
    cmdName.toUpperCase();

    if (cmdName == "HELP") {
        printHelp();
        Serial.println("{\"type\":\"done\"}");
        return true;
    } else if (cmdName == "DUMP_USERS") {
        dumpUsers();
        Serial.println("{\"type\":\"done\"}");
        return true;
    } else if (cmdName == "DUMP_LOGS") {
        dumpLogs();
        Serial.println("{\"type\":\"done\"}");
        return true;
    } else if (cmdName == "CLEAR_LOGS") {
        clearLogs();
        Serial.println("{\"type\":\"done\"}");
        return true;
    } else if (cmdName == "STATUS") {
        printStatus();
        Serial.println("{\"type\":\"done\"}");
        return true;
    } else if (cmdName == "ADD_USER") {
        if (spaceIdx == -1) {
            Serial.println("{\"type\":\"error\",\"message\":\"Missing JSON payload\"}");
            Serial.println("{\"type\":\"done\"}");
            return true;
        }
        String jsonPayload = cmd.substring(spaceIdx + 1);
        addUser(jsonPayload);
        Serial.println("{\"type\":\"done\"}");
        return true;
    } else if (cmdName == "DELETE_USER") {
        if (spaceIdx == -1) {
            Serial.println("{\"type\":\"error\",\"message\":\"Missing UID\"}");
            Serial.println("{\"type\":\"done\"}");
            return true;
        }
        String uid = cmd.substring(spaceIdx + 1);
        deleteUser(uid);
        Serial.println("{\"type\":\"done\"}");
        return true;
    } else if (cmdName == "SET_TIME" || cmdName == "SYNC_TIME") {
        if (spaceIdx == -1) {
            Serial.println("{\"type\":\"error\",\"message\":\"Missing epoch timestamp\"}");
            Serial.println("{\"type\":\"done\"}");
            return true;
        }
        String argStr = upperCmd.substring(spaceIdx + 1);
        argStr.trim();
        int secondSpace = argStr.indexOf(' ');
        unsigned long epoch = 0;
        int tzOffset = 0;
        if (secondSpace != -1) {
            epoch = strtoul(argStr.substring(0, secondSpace).c_str(), nullptr, 10);
            tzOffset = argStr.substring(secondSpace + 1).toInt();
        } else {
            epoch = strtoul(argStr.c_str(), nullptr, 10);
        }
        if (epoch > 1577836800) { // Valid epoch > 2020
            if (_repo) {
                _repo->setClock(epoch, tzOffset);
            }
            Serial.println("{\"type\":\"time_sync\",\"status\":\"OK\",\"epoch\":" + String(epoch) + "}");
        } else {
            Serial.println("{\"type\":\"error\",\"message\":\"Invalid epoch\"}");
        }
        Serial.println("{\"type\":\"done\"}");
        return true;
    }
    return false;
}

void CliManager::printHelp() {
    Serial.println("=== CLI Commands ===");
    Serial.println("HELP       - Show this message");
    Serial.println("DUMP_USERS - Print the raw users.json data");
    Serial.println("DUMP_LOGS  - Stream attendance logs (queue.jsonl)");
    Serial.println("CLEAR_LOGS - Clear all attendance logs");
    Serial.println("====================");
}

void CliManager::dumpUsers() {
    if (!_repo) {
        Serial.println("ERR: No repository.");
        return;
    }
    Serial.println("--- BEGIN USERS ---");
    Serial.println(_repo->getStudents());
    Serial.println("--- END USERS ---");
}

void CliManager::printLogLine(const char* jsonLine, void* context) {
    (void)context;
    if (jsonLine) {
        Serial.println(jsonLine);
    }
}

void CliManager::dumpLogs() {
    if (!_repo) {
        Serial.println("ERR: No repository.");
        return;
    }
    Serial.println("--- BEGIN LOGS ---");
    _repo->forEachAttendance(printLogLine, nullptr);
    Serial.println("--- END LOGS ---");
}

void CliManager::clearLogs() {
    if (!_repo) {
        Serial.println("ERR: No repository.");
        return;
    }
    if (_repo->clearAttendanceLogs()) {
        Serial.println("OK: Logs cleared.");
        LOG_INFO("Attendance logs cleared via CLI");
    } else {
        Serial.println("ERR: Failed to clear logs.");
    }
}

void CliManager::printStatus() {
    JsonDocument doc;
    doc["type"] = "status";
    JsonObject data = doc["data"].to<JsonObject>();
    
#if defined(ESP32)
    data["board"] = "ESP32";
#elif defined(ARDUINO_ARCH_STM32)
    data["board"] = "STM32";
#else
    data["board"] = "Generic";
#endif
    data["firmware"] = "1.0.0";
    
    // Storage status reporting
    data["storage"] = _repo ? _repo->activeBackend() : "RAM";

    // RFID status reporting
    if (_rfid) {
        data["rfid"] = _rfid->isSimulated() ? "SIMULATED" : (_rfid->ready() ? "ONLINE" : "OFFLINE");
    } else {
        data["rfid"] = "UNKNOWN";
    }
    
    if (_repo) {
        String usersJson = _repo->getStudents();
        if (usersJson.length() > 0 && usersJson != "{}") {
            JsonDocument usersDoc;
            deserializeJson(usersDoc, usersJson);
            data["users"] = usersDoc.size();
        } else {
            data["users"] = 0;
        }
        
        struct Counter { int count = 0; };
        Counter c;
        _repo->forEachAttendance([](const char* json, void* ctx) {
            ((Counter*)ctx)->count++;
        }, &c);
        data["logs"] = c.count;
    } else {
        data["users"] = 0;
        data["logs"] = 0;
    }
    
    if (_repo && _repo->isClockSynced()) {
        data["time_synced"] = true;
        time_t epoch = (time_t)_repo->getEpoch();
        struct tm* timeinfo = gmtime(&epoch);
        if (timeinfo) {
            char timeStr[30];
            snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02dT%02d:%02d:%02dZ",
                     timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                     timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
            data["time"] = timeStr;
        }
    } else {
        data["time_synced"] = false;
        data["time"] = "UNSYNCED";
    }
    
    String out;
    serializeJson(doc, out);
    Serial.println(out);
}

void CliManager::addUser(const String& jsonPayload) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, jsonPayload);
    if (err) {
        Serial.println("{\"type\":\"error\",\"message\":\"Malformed JSON\"}");
        return;
    }
    
    if (!doc.containsKey("uid") || !doc.containsKey("name")) {
        Serial.println("{\"type\":\"error\",\"message\":\"Missing uid or name\"}");
        return;
    }
    
    String uid = doc["uid"].as<String>();
    uid.trim();
    uid.toUpperCase();
    String name = doc["name"].as<String>();
    name.trim();
    String role = doc.containsKey("role") ? doc["role"].as<String>() : "User";
    role.trim();
    
    if (uid.length() < 4 || uid.length() > 32) {
        Serial.println("{\"type\":\"error\",\"message\":\"Invalid UID length\"}");
        return;
    }
    for (size_t i = 0; i < uid.length(); i++) {
        if (!isxdigit(uid[i])) {
            Serial.println("{\"type\":\"error\",\"message\":\"UID must be hexadecimal\"}");
            return;
        }
    }
    
    if (name.length() == 0 || name.length() > 80) {
        Serial.println("{\"type\":\"error\",\"message\":\"Invalid name length\"}");
        return;
    }
    
    if (!_repo) {
        Serial.println("{\"type\":\"error\",\"message\":\"No repository\"}");
        return;
    }

    String existingUsers = _repo->getStudents();
    JsonDocument usersDoc;
    deserializeJson(usersDoc, existingUsers);
    if (usersDoc.containsKey(uid)) {
        Serial.println("{\"type\":\"error\",\"message\":\"Duplicate UID\"}");
        return;
    }
    
    if (_repo->addStudent(uid, name)) {
        JsonDocument successDoc;
        successDoc["type"] = "user";
        JsonObject data = successDoc["data"].to<JsonObject>();
        data["uid"] = uid;
        data["name"] = name;
        data["role"] = role;
        String out;
        serializeJson(successDoc, out);
        Serial.println(out);
    } else {
        Serial.println("{\"type\":\"error\",\"message\":\"Storage failed\"}");
    }
}

void CliManager::deleteUser(const String& uid) {
    if (!_repo) {
        Serial.println("{\"type\":\"error\",\"message\":\"No repository\"}");
        return;
    }

    String cleanUid = uid;
    cleanUid.trim();
    cleanUid.toUpperCase();
    
    if (cleanUid.length() < 4 || cleanUid.length() > 32) {
        Serial.println("{\"type\":\"error\",\"message\":\"Invalid UID length\"}");
        return;
    }

    String existingUsers = _repo->getStudents();
    JsonDocument usersDoc;
    deserializeJson(usersDoc, existingUsers);
    
    if (!usersDoc.containsKey(cleanUid)) {
        Serial.println("{\"type\":\"error\",\"message\":\"User not found\"}");
        return;
    }
    
    if (_repo->deleteStudent(cleanUid)) {
        JsonDocument successDoc;
        successDoc["type"] = "deleted";
        successDoc["uid"] = cleanUid;
        String out;
        serializeJson(successDoc, out);
        Serial.println(out);
    } else {
        Serial.println("{\"type\":\"error\",\"message\":\"Storage failed\"}");
    }
}
