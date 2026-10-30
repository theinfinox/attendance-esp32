#pragma once

#include <Arduino.h>
#include "types.h"

class IAttendanceRepository {
public:
    virtual ~IAttendanceRepository() = default;
    
    virtual void init() = 0;

    // Student Management
    virtual bool addStudent(const String& uid, const String& name) = 0;
    virtual bool deleteStudent(const String& uid) = 0;
    virtual String getStudents() = 0;
    
    // For local lookups
    virtual String getStudentName(const String& uid) = 0;

    // Attendance Logging
    virtual bool markAttendance(const String& uid, const String& name, unsigned long timestamp) = 0;
    
    // Advanced Attendance Logging with State Machine
    virtual bool recordPunch(const String& uid, const String& name, unsigned long timestamp,
                             String& outEventType, uint16_t& outSessionId) {
        outEventType = "Check-in";
        outSessionId = 1;
        return markAttendance(uid, name, timestamp);
    }
    
    // Streaming logs (avoids OOM). Callback is invoked for each JSON line.
    virtual void forEachAttendance(void (*callback)(const char* jsonLine, void* context), void* context) = 0;
    
    virtual bool clearAttendanceLogs() = 0;

    virtual const char* activeBackend() const { return "RAM"; }

    // Timekeeping & Calendar
    virtual void setClock(unsigned long epoch, int tzOffsetMinutes = 0) {}
    virtual bool isClockSynced() const { return false; }
    virtual unsigned long getEpoch() const { return 0; }
    virtual uint32_t getActiveDate() const { return 0; }
};
