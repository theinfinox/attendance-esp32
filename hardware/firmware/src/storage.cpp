// ============================================================================
// storage.cpp — Offline Storage & Embedded DB
// ============================================================================

#include "storage.h"
#include "logger.h"
#include "config.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

#define USERS_FILE "/users.json"
#define LOGS_FILE "/logs.json"

void Storage::init() {
    _pendingCount = 0;
    
    if (CFG_STANDALONE_MODE) {
        if (!LittleFS.begin(true)) { // true = format on fail
            LOG_ERROR("LittleFS Mount Failed!");
            return;
        }
        LOG_INFO("LittleFS initialized for Standalone Backend.");
        
        // Ensure files exist
        if (!LittleFS.exists(USERS_FILE)) {
            writeFile(USERS_FILE, "{}"); // Empty dict
        }
        if (!LittleFS.exists(LOGS_FILE)) {
            writeFile(LOGS_FILE, "[]"); // Empty array
        }
    } else {
        LOG_INFO("Storage initialized (stub — no persistent storage yet)");
    }
}

// --- Original Sync Stub Methods ---
bool Storage::storeOfflineRecord(const String& uid, unsigned long timestamp) {
    LOG_WARN("Storage: offline record for UID=%s t=%lu (stub)", uid.c_str(), timestamp);
    _pendingCount++;
    return true;
}
bool Storage::hasOfflineRecords() const { return _pendingCount > 0; }
int Storage::offlineRecordCount() const { return _pendingCount; }
int Storage::flushOfflineRecords() {
    int flushed = _pendingCount;
    _pendingCount = 0;
    return flushed;
}
void Storage::clearAll() { _pendingCount = 0; }

// --- Helpers ---
String Storage::readFile(const char* path) {
    File file = LittleFS.open(path, "r");
    if (!file) return "";
    String content = file.readString();
    file.close();
    return content;
}

bool Storage::writeFile(const char* path, const String& content) {
    File file = LittleFS.open(path, "w");
    if (!file) return false;
    file.print(content);
    file.close();
    return true;
}

// --- Embedded Backend Methods ---

String Storage::getUsersJson() {
    return readFile(USERS_FILE);
}

bool Storage::addCard(const String& uid, const String& name) {
    String json = readFile(USERS_FILE);
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err && json != "{}") return false;
    
    doc[uid] = name;
    
    String output;
    serializeJson(doc, output);
    return writeFile(USERS_FILE, output);
}

bool Storage::removeCard(const String& uid) {
    String json = readFile(USERS_FILE);
    JsonDocument doc;
    deserializeJson(doc, json);
    doc.remove(uid);
    String output;
    serializeJson(doc, output);
    return writeFile(USERS_FILE, output);
}

String Storage::getCardName(const String& uid) {
    String json = readFile(USERS_FILE);
    JsonDocument doc;
    deserializeJson(doc, json);
    if (doc.containsKey(uid)) {
        return doc[uid].as<String>();
    }
    return "";
}

String Storage::getLogsJson() {
    return readFile(LOGS_FILE);
}

bool Storage::clearLogs() {
    return writeFile(LOGS_FILE, "[]");
}

bool Storage::logAttendance(const String& uid, const String& name, unsigned long timestamp) {
    String json = readFile(LOGS_FILE);
    JsonDocument doc; // Array
    deserializeJson(doc, json);
    
    JsonObject entry = doc.add<JsonObject>();
    entry["uid"] = uid;
    entry["name"] = name;
    entry["ts"] = timestamp;
    
    // Keep only the last 50 logs to save memory/flash
    while (doc.size() > 50) {
        doc.remove(0);
    }
    
    String output;
    serializeJson(doc, output);
    return writeFile(LOGS_FILE, output);
}
