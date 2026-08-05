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
    virtual String getStudents() = 0; // Returning JSON string for now to avoid changing WebManager
    
    // For local lookups
    virtual String getStudentName(const String& uid) = 0;

    // Attendance Logging
    virtual bool markAttendance(const String& uid, const String& name, unsigned long timestamp) = 0;
    
    // Streaming logs (avoids OOM). Callback is invoked for each JSON line.
    virtual void forEachAttendance(void (*callback)(const char* jsonLine, void* context), void* context) = 0;
    
    virtual bool clearAttendanceLogs() = 0;
};
