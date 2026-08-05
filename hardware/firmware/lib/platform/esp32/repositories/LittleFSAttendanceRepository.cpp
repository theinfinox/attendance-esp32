// ============================================================================
// storage.cpp — Offline Storage & Embedded DB
// ============================================================================

#include "LittleFSAttendanceRepository.h"
#include "logger.h"
#include "config.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>

#define USERS_FILE "/users.json"
#define LOGS_FILE "/logs.json"

void LittleFSAttendanceRepository::init() {
    _pendingCount = 0;
    
    if (CFG_STANDALONE_MODE) {
        if (!LittleFS.begin(true)) { // true = format on fail
            LOG_ERROR("LittleFS Mount Failed!");
            return;
        }
        LOG_INFO("LittleFS initialized for Standalone Backend.");
        
        // Remove old logs.json array file if it exists (Migration to JSONL)
        if (LittleFS.exists(LOGS_FILE)) {
            LittleFS.remove(LOGS_FILE);
            LOG_INFO("Removed old logs.json array file.");
        }
        
        // Ensure files exist
        if (!LittleFS.exists(USERS_FILE)) {
            writeFile(USERS_FILE, "{}"); // Empty dict
        }
        if (!LittleFS.exists("/queue.jsonl")) {
            writeFile("/queue.jsonl", ""); // Empty JSONL
        }
    } else {
        LOG_INFO("Storage initialized (stub — no persistent storage yet)");
    }
}

// --- Original Sync Stub Methods ---
bool LittleFSAttendanceRepository::storeOfflineRecord(const String& uid, unsigned long timestamp) {
    LOG_WARN("Storage: offline record for UID=%s t=%lu (stub)", uid.c_str(), timestamp);
    _pendingCount++;
    return true;
}
bool LittleFSAttendanceRepository::hasOfflineRecords() const { return _pendingCount > 0; }
int LittleFSAttendanceRepository::offlineRecordCount() const { return _pendingCount; }
int LittleFSAttendanceRepository::flushOfflineRecords() {
    int flushed = _pendingCount;
    _pendingCount = 0;
    return flushed;
}
void LittleFSAttendanceRepository::clearAll() { _pendingCount = 0; }

// --- Helpers ---
String LittleFSAttendanceRepository::readFile(const char* path) {
    File file = LittleFS.open(path, "r");
    if (!file) return "";
    String content = file.readString();
    file.close();
    return content;
}

bool LittleFSAttendanceRepository::writeFile(const char* path, const String& content) {
    File file = LittleFS.open(path, "w");
    if (!file) return false;
    file.print(content);
    file.close();
    return true;
}

// --- Embedded Backend Methods ---

String LittleFSAttendanceRepository::getStudents() {
    return readFile(USERS_FILE);
}

bool LittleFSAttendanceRepository::addStudent(const String& uid, const String& name) {
    String json = readFile(USERS_FILE);
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err && json != "{}") return false;
    
    doc[uid] = name;
    
    String output;
    serializeJson(doc, output);
    
    // Temp file + Rename to prevent corruption on power loss
    if (writeFile("/users.tmp", output)) {
        LittleFS.remove(USERS_FILE);
        return LittleFS.rename("/users.tmp", USERS_FILE);
    }
    return false;
}

bool LittleFSAttendanceRepository::deleteStudent(const String& uid) {
    String json = readFile(USERS_FILE);
    JsonDocument doc;
    deserializeJson(doc, json);
    doc.remove(uid);
    String output;
    serializeJson(doc, output);
    
    // Temp file + Rename
    if (writeFile("/users.tmp", output)) {
        LittleFS.remove(USERS_FILE);
        return LittleFS.rename("/users.tmp", USERS_FILE);
    }
    return false;
}

String LittleFSAttendanceRepository::getStudentName(const String& uid) {
    String json = readFile(USERS_FILE);
    JsonDocument doc;
    deserializeJson(doc, json);
    if (doc.containsKey(uid)) {
        return doc[uid].as<String>();
    }
    return "";
}

void LittleFSAttendanceRepository::forEachAttendance(void (*callback)(const char* jsonLine, void* context), void* context) {
    if (!LittleFS.exists("/queue.jsonl")) return;
    File file = LittleFS.open("/queue.jsonl", "r");
    if (!file) return;
    
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        // Skip malformed lines
        if (line.length() > 0 && line.startsWith("{") && line.endsWith("}")) {
            callback(line.c_str(), context);
        }
    }
    file.close();
}

bool LittleFSAttendanceRepository::clearAttendanceLogs() {
    return writeFile("/queue.jsonl", "");
}

bool LittleFSAttendanceRepository::markAttendance(const String& uid, const String& name, unsigned long timestamp) {
    time_t t = (time_t)timestamp;
    struct tm* timeinfo = localtime(&t);
    char timeStr[25];
    
    // If year > 100 (after year 2000), time is valid via NTP
    if (timeinfo->tm_year > 100) {
        snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02dT%02d:%02d:%02d",
                 timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
                 timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    } else {
        snprintf(timeStr, sizeof(timeStr), "%lu", timestamp); // Fallback to raw UNIX stamp if no NTP
    }

    JsonDocument doc;
    doc["uid"] = uid;
    doc["name"] = name;
    doc["time"] = timeStr;
    doc["status"] = "IN";
    
    String output;
    serializeJson(doc, output);
    output += "\n";
    
    // Append-only JSONL prevents OOM and minimizes flash wear/corruption risks
    File file = LittleFS.open("/queue.jsonl", "a");
    if (!file) return false;
    file.print(output);
    file.close();
    return true;
}
