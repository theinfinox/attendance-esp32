// ============================================================================
// STM32FlashRepository.cpp — Pure On-Chip Flash Attendance & Student Repository
// 3-Zone Architecture: Zone A (Users), Zone B (Active State), Zone C (Journal)
// ============================================================================

#include "STM32FlashRepository.h"
#include "logger.h"
#include <time.h>

void STM32FlashRepository::init() {
    LOG_INFO("STM32FlashRepo: Initializing 3-Zone Flash Storage...");
    
    bool usersOk = _userStore.init();
    bool stateOk = _stateStore.init();
    bool journalOk = _journal.init();

    if (usersOk && stateOk && journalOk) {
        LOG_INFO("STM32FlashRepo: Ready. Enrolled=%u, CheckedIn=%u, Journal=%u/853",
                 (unsigned int)_userStore.getUserCount(),
                 (unsigned int)_stateStore.getCheckedInCount(),
                 (unsigned int)_journal.getTotalRecordCount());
        reconcileBootState();
    } else {
        LOG_ERROR("STM32FlashRepo: Init failed! UsersOk=%d, StateOk=%d, JournalOk=%d",
                  usersOk, stateOk, journalOk);
    }
}

void STM32FlashRepository::reconcileBootState() {
    uint32_t journalHighest = _journal.getHighestEventId();
    uint32_t stateHighest = _stateStore.getHighestCheckpointEventId();

    if (journalHighest > stateHighest) {
        LOG_WARN("STM32FlashRepo: Reconciliation needed. Zone C Highest=%lu, Zone B Highest=%lu",
                 journalHighest, stateHighest);
        
        struct ReconcileContext {
            STM32FlashStateStore* stateStore;
            uint32_t threshold;
        } ctx = { &_stateStore, stateHighest };

        _journal.forEachRecord([](const FlashAttendanceEntry& entry, void* c) {
            ReconcileContext* rc = static_cast<ReconcileContext*>(c);
            if (entry.event_id > rc->threshold) {
                rc->stateStore->reconcileEvent(entry.uid, entry.event_type, entry.session_id,
                                               entry.event_id, (uint32_t)entry.timestamp, entry.attendance_date);
            }
        }, &ctx);
        LOG_INFO("STM32FlashRepo: State reconciled up to Event #%lu", journalHighest);
    }
}

bool STM32FlashRepository::addStudent(const String& uid, const String& name) {
    return _userStore.addUser(uid, name);
}

bool STM32FlashRepository::deleteStudent(const String& uid) {
    return _userStore.deleteUser(uid);
}

String STM32FlashRepository::getStudentName(const String& uid) {
    return _userStore.getUserName(uid);
}

String STM32FlashRepository::getStudents() {
    return _userStore.getAllUsersJson();
}

bool STM32FlashRepository::markAttendance(const String& uid, const String& name, unsigned long timestamp) {
    String dummyEvent;
    uint16_t dummySession;
    return recordPunch(uid, name, timestamp, dummyEvent, dummySession);
}

bool STM32FlashRepository::recordPunch(const String& uid, const String& name, unsigned long timestamp,
                                      String& outEventType, uint16_t& outSessionId) {
    if (_journal.isFull()) {
        LOG_WARN("STM32FlashRepo: Attendance rejected — Flash journal is FULL (853/853 records).");
        return false;
    }

    uint32_t activeDate = getActiveDate();
    _stateStore.checkDateRollover(activeDate);

    // 1. Determine next transition state
    uint8_t currSession = 0;
    uint8_t currState = _stateStore.getStudentState(uid, currSession);

    uint8_t eventType = EVENT_TYPE_CHECK_IN;
    uint8_t nextState = RECORD_STATE_CHECKED_IN;

    if (currState == RECORD_STATE_CHECKED_IN) {
        eventType = EVENT_TYPE_CHECK_OUT;
        nextState = RECORD_STATE_CHECKED_OUT;
        outEventType = "Check-out";
        outSessionId = currSession; // Same session ID for matching checkout
    } else {
        eventType = EVENT_TYPE_CHECK_IN;
        nextState = RECORD_STATE_CHECKED_IN;
        outEventType = "Check-in";
        outSessionId = currSession + 1; // Increment session counter
    }

    uint16_t flags = _clockSynced ? RECORD_FLAG_NORMAL : RECORD_FLAG_UNSYNCED;

    // 2. Phase 1: Commit Event Record to Zone C Journal
    uint32_t eventId = 0;
    if (!_journal.logPunch(uid, name, (uint64_t)timestamp, activeDate,
                           eventType, (uint8_t)outSessionId, flags, eventId)) {
        LOG_ERROR("STM32FlashRepo: Failed to log event to Zone C.");
        return false;
    }

    // 3. Phase 2: Checkpoint Active State to Zone B
    if (!_stateStore.checkpointTransition(uid, nextState, (uint8_t)outSessionId,
                                          eventId, (uint32_t)timestamp, activeDate, flags)) {
        LOG_ERROR("STM32FlashRepo: Failed to checkpoint state in Zone B.");
        // Still return true because Event is safely persisted in Zone C and will reconcile
    }

    LOG_INFO("STM32FlashRepo: Recorded %s for %s (Session #%u, Event #%lu)",
             outEventType.c_str(), name.c_str(), outSessionId, eventId);
    return true;
}

void STM32FlashRepository::forEachAttendance(void (*callback)(const char* jsonLine, void* context), void* context) {
    if (!callback) return;

    struct IterContext {
        STM32FlashRepository* repo;
        void (*cb)(const char*, void*);
        void* userCtx;
    };
    IterContext ctx = { this, callback, context };

    _journal.forEachRecord([](const FlashAttendanceEntry& entry, void* c) {
        IterContext* ic = static_cast<IterContext*>(c);
        JsonDocument doc;
        doc["uid"] = entry.uid;
        
        String fullName = ic->repo->_userStore.getUserName(entry.uid);
        doc["name"] = (fullName.length() > 0) ? fullName : String(entry.name_cache);
        doc["timestamp"] = (unsigned long)entry.timestamp;
        
        if (entry.event_type == EVENT_TYPE_CHECK_OUT) {
            doc["event"] = "Check-out";
            doc["status"] = "OUT";
        } else {
            doc["event"] = "Check-in";
            doc["status"] = "IN";
        }
        
        doc["session"] = entry.session_id;
        doc["event_id"] = entry.event_id;
        
        if (entry.attendance_date != 0) {
            doc["date"] = entry.attendance_date;
        }

        String line;
        serializeJson(doc, line);
        ic->cb(line.c_str(), ic->userCtx);
    }, &ctx);
}

bool STM32FlashRepository::clearAttendanceLogs() {
    // Erases ONLY Zone C (Journal). Zone B (Active State) is 100% PRESERVED!
    return _journal.clearJournal();
}

void STM32FlashRepository::setClock(unsigned long epoch, int tzOffsetMinutes) {
    _baseEpoch = epoch;
    _syncMillis = millis();
    _tzOffsetMinutes = tzOffsetMinutes;
    _clockSynced = true;

    uint32_t activeDate = calculateDateFromEpoch(epoch);
    _stateStore.setActiveDate(activeDate);
    _stateStore.checkDateRollover(activeDate);
    LOG_INFO("STM32FlashRepo: Clock calibrated to epoch=%lu (TZ=%d min, Date=%lu)",
             epoch, tzOffsetMinutes, activeDate);
}

unsigned long STM32FlashRepository::getEpoch() const {
    if (_clockSynced) {
        return _baseEpoch + ((millis() - _syncMillis) / 1000);
    }
    return (millis() / 1000);
}

uint32_t STM32FlashRepository::getActiveDate() const {
    if (_clockSynced) {
        return calculateDateFromEpoch(getEpoch());
    }
    return _stateStore.getActiveDate();
}

uint32_t STM32FlashRepository::calculateDateFromEpoch(unsigned long epoch) const {
    unsigned long localEpoch = epoch + (_tzOffsetMinutes * 60);
    time_t t = (time_t)localEpoch;
    struct tm* tmInfo = gmtime(&t);
    if (!tmInfo) return 0;

    uint32_t y = tmInfo->tm_year + 1900;
    uint32_t m = tmInfo->tm_mon + 1;
    uint32_t d = tmInfo->tm_mday;
    return (y * 10000) + (m * 100) + d;
}
