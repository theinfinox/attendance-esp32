#pragma once
// ============================================================================
// STM32AttendanceRepository.h — STM32 Storage Implementation
// Supports SD Card storage (SD.h) with in-RAM volatile fallback when no SD card is mounted.
// ============================================================================

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "IAttendanceRepository.h"
#include "constants.h"
#include "config.h"

class STM32AttendanceRepository : public IAttendanceRepository {
public:
    void init() override;

    bool addStudent(const String& uid, const String& name) override;
    bool deleteStudent(const String& uid) override;
    String getStudentName(const String& uid) override;
    String getStudents() override;

    bool markAttendance(const String& uid, const String& name, unsigned long timestamp) override;
    void forEachAttendance(void (*callback)(const char* jsonLine, void* context), void* context) override;
    bool clearAttendanceLogs() override;

    bool hasSD() const { return _hasSD; }

private:
    bool _hasSD = false;

    // RAM Fallback storage
    String _ramUsersJson = "{}";
    String _ramLogsJsonl = "";

    // SD File helpers
    String readFile(const char* path);
    bool writeFile(const char* path, const String& content);
};

