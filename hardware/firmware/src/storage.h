#pragma once
// ============================================================================
// storage.h — Offline Storage & Embedded DB
// Handles LittleFS persistence for standalone backend mode.
// ============================================================================

#include <Arduino.h>

class Storage {
public:
    void init();

    // --- Original Sync Stub Methods ---
    bool storeOfflineRecord(const String& uid, unsigned long timestamp);
    bool hasOfflineRecords() const;
    int offlineRecordCount() const;
    int flushOfflineRecords();
    void clearAll();

    // --- Embedded Backend Methods ---
    // Users database
    bool addCard(const String& uid, const String& name);
    bool removeCard(const String& uid);
    String getCardName(const String& uid);
    String getUsersJson();

    // Attendance Logs database
    bool logAttendance(const String& uid, const String& name, unsigned long timestamp);
    String getLogsJson();
    bool clearLogs();

private:
    int _pendingCount = 0;

    // LittleFS helpers
    String readFile(const char* path);
    bool writeFile(const char* path, const String& content);
};
