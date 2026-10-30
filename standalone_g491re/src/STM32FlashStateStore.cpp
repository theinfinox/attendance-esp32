// ============================================================================
// STM32FlashStateStore.cpp — 8KB Active Attendance State Store in Internal Flash
// Ping-Pong Compacting State Journal across Pages 232..235 (Zone B)
// NEVER erased by CLEAR_LOGS. Tracks active IN/OUT status for current date.
// ============================================================================

#include "STM32FlashStateStore.h"
#include "STM32FlashDriver.h"
#include "logger.h"

int STM32FlashStateStore::findCacheIndex(const String& uid) {
    for (size_t i = 0; i < _activeCacheCount; ++i) {
        if (strcasecmp(_activeCache[i].uid, uid.c_str()) == 0) {
            return (int)i;
        }
    }
    return -1;
}

bool STM32FlashStateStore::loadBank(uint32_t bankAddr, FlashStateBankHeader& header) {
    STM32FlashDriver::readBuffer(bankAddr, &header, sizeof(header));
    if (header.magic != FLASH_STATE_MAGIC) {
        return false;
    }
    uint16_t calcCrc = calculateCRC16(reinterpret_cast<const uint8_t*>(&header), sizeof(header) - 2);
    return (calcCrc == header.crc16);
}

bool STM32FlashStateStore::init() {
    _activeCacheCount = 0;
    _highestEventId = 0;
    _transitionCount = 0;

    FlashStateBankHeader header0, header1;
    bool valid0 = loadBank(FLASH_STATE_BANK_0_ADDR, header0);
    bool valid1 = loadBank(FLASH_STATE_BANK_1_ADDR, header1);

    if (valid0 && valid1) {
        if (header1.generation > header0.generation) {
            _activeBankAddr = FLASH_STATE_BANK_1_ADDR;
            _activeGeneration = header1.generation;
            _activeDate = header1.active_date;
        } else {
            _activeBankAddr = FLASH_STATE_BANK_0_ADDR;
            _activeGeneration = header0.generation;
            _activeDate = header0.active_date;
        }
    } else if (valid0) {
        _activeBankAddr = FLASH_STATE_BANK_0_ADDR;
        _activeGeneration = header0.generation;
        _activeDate = header0.active_date;
    } else if (valid1) {
        _activeBankAddr = FLASH_STATE_BANK_1_ADDR;
        _activeGeneration = header1.generation;
        _activeDate = header1.active_date;
    } else {
        // Initial fresh provisioning
        LOG_INFO("STM32FlashStateStore: Fresh initialization of Zone B (Pages 232..235)");
        uint32_t page0 = STM32FlashDriver::addressToPage(FLASH_STATE_BANK_0_ADDR);
        STM32FlashDriver::erasePages(page0, 2);
        
        FlashStateBankHeader newHdr;
        memset(&newHdr, 0, sizeof(newHdr));
        newHdr.magic = FLASH_STATE_MAGIC;
        newHdr.generation = 1;
        newHdr.active_date = 0;
        newHdr.state_count = 0;
        newHdr.crc16 = calculateCRC16(reinterpret_cast<const uint8_t*>(&newHdr), sizeof(newHdr) - 2);
        
        STM32FlashDriver::writeBuffer(FLASH_STATE_BANK_0_ADDR, &newHdr, sizeof(newHdr));
        _activeBankAddr = FLASH_STATE_BANK_0_ADDR;
        _activeGeneration = 1;
        _activeDate = 0;
    }

    _nextWriteAddr = _activeBankAddr + sizeof(FlashStateBankHeader);
    bool foundBlank = false;

    // Scan bank transition records
    FlashStateTransitionRecord rec;
    for (size_t i = 0; i < FLASH_MAX_STATE_TRANSITIONS; ++i) {
        uint32_t slotAddr = _activeBankAddr + sizeof(FlashStateBankHeader) + (i * sizeof(FlashStateTransitionRecord));
        STM32FlashDriver::readBuffer(slotAddr, &rec, sizeof(rec));

        if (rec.marker == RECORD_MARKER_ERASED) {
            if (!foundBlank) {
                _nextWriteAddr = slotAddr;
                foundBlank = true;
            }
        } else if (rec.marker == RECORD_MARKER_COMMITTED) {
            uint16_t calcCrc = calculateCRC16(reinterpret_cast<const uint8_t*>(&rec), sizeof(rec) - 2);
            if (calcCrc == rec.crc16) {
                _transitionCount++;
                if (rec.event_id > _highestEventId) {
                    _highestEventId = rec.event_id;
                }
                
                // Reconstruct UID string
                char fullUid[16];
                memset(fullUid, 0, sizeof(fullUid));
                strncpy(fullUid, rec.uid, 8);
                strncat(fullUid, rec.uid_ext, 4);

                int idx = findCacheIndex(fullUid);
                if (idx == -1 && _activeCacheCount < FLASH_MAX_USERS) {
                    idx = (int)_activeCacheCount++;
                    strncpy(_activeCache[idx].uid, fullUid, sizeof(_activeCache[idx].uid) - 1);
                }
                if (idx != -1) {
                    _activeCache[idx].state = rec.state;
                    _activeCache[idx].session_id = rec.session_id;
                    _activeCache[idx].last_event_id = rec.event_id;
                    _activeCache[idx].timestamp = rec.timestamp;
                    _activeCache[idx].date = rec.date;
                }
            } else {
                LOG_WARN("STM32FlashStateStore: Corrupt transition slot at 0x%08lX", slotAddr);
            }
        }
    }

    if (!foundBlank) {
        _nextWriteAddr = _activeBankAddr + FLASH_STATE_BANK_SIZE;
    }

    LOG_INFO("STM32FlashStateStore: Active Bank=0x%08lX (Gen=%lu, Date=%lu), Transitions=%u/%u, CheckedIn=%u",
             _activeBankAddr, _activeGeneration, _activeDate, _transitionCount,
             (unsigned int)FLASH_MAX_STATE_TRANSITIONS, (unsigned int)getCheckedInCount());
    return true;
}

uint8_t STM32FlashStateStore::getStudentState(const String& uid, uint8_t& outSessionId) {
    int idx = findCacheIndex(uid);
    if (idx == -1) {
        outSessionId = 0;
        return RECORD_STATE_CHECKED_OUT;
    }

    // Check if entry belongs to active date
    if (_activeDate != 0 && _activeCache[idx].date != _activeDate) {
        // Belongs to previous day — per policy, evaluate as no attendance today
        outSessionId = 0;
        return RECORD_STATE_CHECKED_OUT;
    }

    outSessionId = _activeCache[idx].session_id;
    return _activeCache[idx].state;
}

void STM32FlashStateStore::checkDateRollover(uint32_t newDate) {
    if (newDate != 0 && newDate != _activeDate) {
        LOG_INFO("STM32FlashStateStore: Rollover from Date %lu to %lu", _activeDate, newDate);
        _activeDate = newDate;
        // Invalidate active session states for the new day
        for (size_t i = 0; i < _activeCacheCount; ++i) {
            _activeCache[i].state = RECORD_STATE_CHECKED_OUT;
            _activeCache[i].session_id = 0;
            _activeCache[i].date = newDate;
        }
    }
}

bool STM32FlashStateStore::checkpointTransition(const String& uid, uint8_t newState, uint8_t sessionId,
                                                uint32_t eventId, uint32_t timestamp, uint32_t date, uint16_t flags) {
    if (date != 0 && date != _activeDate) {
        checkDateRollover(date);
    }

    // Check if active bank is full
    if (_transitionCount >= FLASH_MAX_STATE_TRANSITIONS ||
        _nextWriteAddr >= (_activeBankAddr + FLASH_STATE_BANK_SIZE)) {
        LOG_INFO("STM32FlashStateStore: Active bank full (%u transitions). Executing compaction...", _transitionCount);
        if (!compactBank()) {
            LOG_ERROR("STM32FlashStateStore: Compaction failed!");
            return false;
        }
    }

    String cleanUid = uid; cleanUid.trim(); cleanUid.toUpperCase();

    FlashStateTransitionRecord rec;
    memset(&rec, 0, sizeof(rec));
    rec.marker = RECORD_MARKER_COMMITTED;
    rec.state = newState;
    rec.session_id = sessionId;
    rec.event_id = eventId;
    rec.timestamp = timestamp;
    rec.date = (date != 0) ? date : _activeDate;
    rec.flags = flags;

    // Split UID into uid[8] and uid_ext[4]
    strncpy(rec.uid, cleanUid.c_str(), 8);
    rec.uid[7] = '\0';
    if (cleanUid.length() > 7) {
        strncpy(rec.uid_ext, cleanUid.c_str() + 7, 4);
        rec.uid_ext[3] = '\0';
    }

    rec.crc16 = calculateCRC16(reinterpret_cast<const uint8_t*>(&rec), sizeof(rec) - 2);

    if (!STM32FlashDriver::writeBuffer(_nextWriteAddr, &rec, sizeof(rec))) {
        LOG_ERROR("STM32FlashStateStore: Write transition failed at 0x%08lX", _nextWriteAddr);
        return false;
    }

    _transitionCount++;
    _nextWriteAddr += sizeof(FlashStateTransitionRecord);
    if (eventId > _highestEventId) {
        _highestEventId = eventId;
    }

    // Update RAM cache
    int idx = findCacheIndex(cleanUid);
    if (idx == -1 && _activeCacheCount < FLASH_MAX_USERS) {
        idx = (int)_activeCacheCount++;
        strncpy(_activeCache[idx].uid, cleanUid.c_str(), sizeof(_activeCache[idx].uid) - 1);
    }
    if (idx != -1) {
        _activeCache[idx].state = newState;
        _activeCache[idx].session_id = sessionId;
        _activeCache[idx].last_event_id = eventId;
        _activeCache[idx].timestamp = timestamp;
        _activeCache[idx].date = rec.date;
    }

    LOG_INFO("STM32FlashStateStore: Checkpointed %s -> State=%s (Session %u) at 0x%08lX",
             cleanUid.c_str(), (newState == RECORD_STATE_CHECKED_IN) ? "IN" : "OUT", sessionId, _nextWriteAddr - sizeof(rec));
    return true;
}

bool STM32FlashStateStore::compactBank() {
    uint32_t standbyAddr = (_activeBankAddr == FLASH_STATE_BANK_0_ADDR) ? FLASH_STATE_BANK_1_ADDR : FLASH_STATE_BANK_0_ADDR;
    uint32_t standbyPage = STM32FlashDriver::addressToPage(standbyAddr);

    if (!STM32FlashDriver::erasePages(standbyPage, 2)) {
        LOG_ERROR("STM32FlashStateStore: Failed to erase standby bank at 0x%08lX", standbyAddr);
        return false;
    }

    FlashStateBankHeader newHdr;
    memset(&newHdr, 0, sizeof(newHdr));
    newHdr.magic = FLASH_STATE_MAGIC;
    newHdr.generation = _activeGeneration + 1;
    newHdr.active_date = _activeDate;
    newHdr.state_count = 0;

    uint32_t writePtr = standbyAddr + sizeof(FlashStateBankHeader);
    uint16_t carriedForward = 0;

    // Carry forward ONLY students currently CHECKED_IN
    for (size_t i = 0; i < _activeCacheCount; ++i) {
        if (_activeCache[i].state == RECORD_STATE_CHECKED_IN &&
            (_activeDate == 0 || _activeCache[i].date == _activeDate)) {
            
            FlashStateTransitionRecord snap;
            memset(&snap, 0, sizeof(snap));
            snap.marker = RECORD_MARKER_COMMITTED;
            snap.state = _activeCache[i].state;
            snap.session_id = _activeCache[i].session_id;
            snap.event_id = _activeCache[i].last_event_id;
            snap.timestamp = _activeCache[i].timestamp;
            snap.date = _activeCache[i].date;
            snap.flags = RECORD_FLAG_NORMAL;

            strncpy(snap.uid, _activeCache[i].uid, 8);
            snap.uid[7] = '\0';
            if (strlen(_activeCache[i].uid) > 7) {
                strncpy(snap.uid_ext, _activeCache[i].uid + 7, 4);
                snap.uid_ext[3] = '\0';
            }

            snap.crc16 = calculateCRC16(reinterpret_cast<const uint8_t*>(&snap), sizeof(snap) - 2);
            STM32FlashDriver::writeBuffer(writePtr, &snap, sizeof(snap));
            writePtr += sizeof(FlashStateTransitionRecord);
            carriedForward++;
        }
    }

    newHdr.state_count = carriedForward;
    newHdr.crc16 = calculateCRC16(reinterpret_cast<const uint8_t*>(&newHdr), sizeof(newHdr) - 2);
    STM32FlashDriver::writeBuffer(standbyAddr, &newHdr, sizeof(newHdr));

    // Erase old bank
    uint32_t oldPage = STM32FlashDriver::addressToPage(_activeBankAddr);
    STM32FlashDriver::erasePages(oldPage, 2);

    _activeBankAddr = standbyAddr;
    _activeGeneration++;
    _transitionCount = carriedForward;
    _nextWriteAddr = writePtr;

    LOG_INFO("STM32FlashStateStore: Compaction complete. New Bank=0x%08lX (Gen=%lu), Carried=%u",
             _activeBankAddr, _activeGeneration, carriedForward);
    return true;
}

void STM32FlashStateStore::reconcileEvent(const String& uid, uint8_t eventType, uint8_t sessionId,
                                         uint32_t eventId, uint32_t timestamp, uint32_t date) {
    if (eventId <= _highestEventId) {
        return; // Already checkpointed
    }
    uint8_t targetState = (eventType == EVENT_TYPE_CHECK_IN) ? RECORD_STATE_CHECKED_IN : RECORD_STATE_CHECKED_OUT;
    checkpointTransition(uid, targetState, sessionId, eventId, timestamp, date, RECORD_FLAG_NORMAL);
}

size_t STM32FlashStateStore::getCheckedInCount() const {
    size_t count = 0;
    for (size_t i = 0; i < _activeCacheCount; ++i) {
        if (_activeCache[i].state == RECORD_STATE_CHECKED_IN &&
            (_activeDate == 0 || _activeCache[i].date == _activeDate)) {
            count++;
        }
    }
    return count;
}

