#pragma once
// ============================================================================
// STM32FlashJournal.h — 40KB Append-Only Attendance Event Journal in Flash
// Zone C: Pages 236 to 255 (0x08076000 to 0x0807FFFF, 40,960 bytes)
// Strict Capacity Guard: Stops at 853 records, never overwrites
// ERASED WHEN CLEAR_LOGS RUNS (leaves Zone A and Zone B untouched)
// ============================================================================

#include <Arduino.h>
#include "flash_storage.h"

class STM32FlashJournal {
public:
    bool init();
    
    // Logs punch to next blank slot. Returns false if storage is full.
    bool logPunch(const String& uid, const String& name, uint64_t timestamp, uint32_t date,
                  uint8_t eventType, uint8_t sessionId, uint16_t flags, uint32_t& outEventId);
    
    // Iterator for log streaming (PWA DUMP_LOGS)
    void forEachRecord(void (*callback)(const FlashAttendanceEntry& entry, void* context), void* context);
    
    // Clears all 20 journal pages and resets write pointer
    bool clearJournal();

    size_t getTotalRecordCount() const { return _totalRecords; }
    uint32_t getHighestEventId() const { return _highestRecordId; }
    bool isFull() const { return _totalRecords >= FLASH_MAX_ATTENDANCE_RECORDS; }

private:
    uint32_t _nextWriteAddr   = FLASH_JOURNAL_BASE_ADDR + sizeof(FlashJournalHeader);
    uint32_t _highestRecordId = 0;
    size_t   _totalRecords    = 0;
};
