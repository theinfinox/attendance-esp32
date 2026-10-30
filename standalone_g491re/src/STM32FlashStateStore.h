#pragma once
// ============================================================================
// STM32FlashStateStore.h — 8KB Active Attendance State Store in Internal Flash
// Ping-Pong Compacting State Journal across Pages 232..235 (Zone B)
// NEVER erased by CLEAR_LOGS. Tracks active IN/OUT status for current date.
// ============================================================================

#include <Arduino.h>
#include "flash_storage.h"

struct ActiveStudentState {
    char     uid[16];
    uint8_t  state;        // RECORD_STATE_CHECKED_OUT (0) or RECORD_STATE_CHECKED_IN (1)
    uint8_t  session_id;   // Session counter today (1, 2, 3...)
    uint32_t last_event_id;// Associated event ID in Zone C
    uint32_t timestamp;    // Epoch timestamp of last transition
    uint32_t date;         // YYYYMMDD
};

class STM32FlashStateStore {
public:
    STM32FlashStateStore() = default;

    bool init();

    // Query active state
    uint8_t getStudentState(const String& uid, uint8_t& outSessionId);
    
    // Checkpoint a new state transition (commits to Flash)
    bool checkpointTransition(const String& uid, uint8_t newState, uint8_t sessionId,
                              uint32_t eventId, uint32_t timestamp, uint32_t date, uint16_t flags);

    // Date rollover handling
    void checkDateRollover(uint32_t newDate);

    // Boot reconciliation
    uint32_t getHighestCheckpointEventId() const { return _highestEventId; }
    void reconcileEvent(const String& uid, uint8_t eventType, uint8_t sessionId,
                        uint32_t eventId, uint32_t timestamp, uint32_t date);

    uint32_t getActiveDate() const { return _activeDate; }
    void setActiveDate(uint32_t d) { _activeDate = d; }

    size_t getCheckedInCount() const;

private:
    uint32_t _activeBankAddr = FLASH_STATE_BANK_0_ADDR;
    uint32_t _nextWriteAddr = FLASH_STATE_BANK_0_ADDR;
    uint32_t _activeGeneration = 0;
    uint32_t _activeDate = 0;
    uint32_t _highestEventId = 0;
    uint16_t _transitionCount = 0;

    ActiveStudentState _activeCache[FLASH_MAX_USERS];
    size_t _activeCacheCount = 0;

    bool loadBank(uint32_t bankAddr, FlashStateBankHeader& header);
    bool compactBank();
    int findCacheIndex(const String& uid);
};

