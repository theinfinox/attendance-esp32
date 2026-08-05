#pragma once
// ============================================================================
// storage.h — Offline Storage & Embedded DB
// Handles LittleFS persistence for standalone backend mode.
// ============================================================================

#include <Arduino.h>
#include "IAttendanceRepository.h"

class LittleFSAttendanceRepository : public IAttendanceRepository {
public:
    void init() override;

    // --- Original Sync Stub Methods ---
    bool storeOfflineRecord(const String& uid, unsigned long timestamp);
    bool hasOfflineRecords() const;
    int offlineRecordCount() const;
    int flushOfflineRecords();
    void clearAll();

    // --- Embedded Backend Methods ---
    // Users database
    bool addStudent(const String& uid, const String& name) override;
    bool deleteStudent(const String& uid) override;
    String getStudentName(const String& uid) override;
    String getStudents() override;

    // Attendance Logs database
    bool markAttendance(const String& uid, const String& name, unsigned long timestamp) override;
    void forEachAttendance(void (*callback)(const char* jsonLine, void* context), void* context) override;
    bool clearAttendanceLogs() override;

private:
    int _pendingCount = 0;

    // LittleFS helpers
    String readFile(const char* path);
    bool writeFile(const char* path, const String& content);
};
