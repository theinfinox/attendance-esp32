#pragma once
// ============================================================================
// STM32FlashRepository.h — Pure On-Chip Flash Attendance & Student Repository
// 3-Zone Architecture: Zone A (Users), Zone B (Active State), Zone C (Journal)
// ============================================================================

#include <Arduino.h>
#include <ArduinoJson.h>
#include "IAttendanceRepository.h"
#include "STM32FlashUserStore.h"
#include "STM32FlashStateStore.h"
#include "STM32FlashJournal.h"

class STM32FlashRepository : public IAttendanceRepository {
public:
    void init() override;

    bool addStudent(const String& uid, const String& name) override;
    bool deleteStudent(const String& uid) override;
    String getStudentName(const String& uid) override;
    String getStudents() override;

    bool markAttendance(const String& uid, const String& name, unsigned long timestamp) override;
    bool recordPunch(const String& uid, const String& name, unsigned long timestamp,
                     String& outEventType, uint16_t& outSessionId) override;

    void forEachAttendance(void (*callback)(const char* jsonLine, void* context), void* context) override;
    bool clearAttendanceLogs() override;

    const char* activeBackend() const override { return "FLASH"; }

    // Timekeeping & Calendar
    void setClock(unsigned long epoch, int tzOffsetMinutes = 0) override;
    bool isClockSynced() const override { return _clockSynced; }
    unsigned long getEpoch() const override;
    uint32_t getActiveDate() const override;

    bool isStorageFull() const { return _journal.isFull(); }
    size_t getLogCount() const { return _journal.getTotalRecordCount(); }
    size_t getUserCount() const { return _userStore.getUserCount(); }
    size_t getCheckedInCount() const { return _stateStore.getCheckedInCount(); }

private:
    STM32FlashUserStore  _userStore;
    STM32FlashStateStore _stateStore;
    STM32FlashJournal    _journal;

    bool _clockSynced = false;
    unsigned long _baseEpoch = 0;
    unsigned long _syncMillis = 0;
    int _tzOffsetMinutes = 0;

    void reconcileBootState();
    uint32_t calculateDateFromEpoch(unsigned long epoch) const;
};
