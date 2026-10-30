// ============================================================================
// STM32FlashJournal.cpp — 40KB Append-Only Attendance Event Journal in Flash
// Zone C: Pages 236 to 255 (0x08076000 to 0x0807FFFF, 40,960 bytes)
// Strict Capacity Guard: Stops at 853 records, never overwrites
// ============================================================================

#include "STM32FlashJournal.h"
#include "STM32FlashDriver.h"
#include "logger.h"

bool STM32FlashJournal::init() {
    _totalRecords = 0;
    _highestRecordId = 0;
    _nextWriteAddr = FLASH_JOURNAL_BASE_ADDR + sizeof(FlashJournalHeader);

    // 1. Verify or initialize Zone C Journal Header
    FlashJournalHeader hdr;
    STM32FlashDriver::readBuffer(FLASH_JOURNAL_BASE_ADDR, &hdr, sizeof(hdr));
    uint16_t calcHdrCrc = calculateCRC16(reinterpret_cast<const uint8_t*>(&hdr), sizeof(hdr) - 2);

    if (hdr.magic != FLASH_JOURNAL_MAGIC || calcHdrCrc != hdr.crc16) {
        LOG_INFO("STM32FlashJournal: Provisioning fresh Journal Header in Zone C...");
        uint32_t startPage = STM32FlashDriver::addressToPage(FLASH_JOURNAL_BASE_ADDR);
        STM32FlashDriver::erasePages(startPage, FLASH_JOURNAL_TOTAL_PAGES);

        memset(&hdr, 0, sizeof(hdr));
        hdr.magic = FLASH_JOURNAL_MAGIC;
        hdr.format_version = 1;
        hdr.total_capacity = FLASH_MAX_ATTENDANCE_RECORDS;
        hdr.crc16 = calculateCRC16(reinterpret_cast<const uint8_t*>(&hdr), sizeof(hdr) - 2);
        STM32FlashDriver::writeBuffer(FLASH_JOURNAL_BASE_ADDR, &hdr, sizeof(hdr));
    }

    // 2. Scan record slots
    FlashAttendanceEntry entry;
    bool foundBlank = false;

    for (size_t i = 0; i < FLASH_MAX_ATTENDANCE_RECORDS; ++i) {
        uint32_t addr = FLASH_JOURNAL_BASE_ADDR + sizeof(FlashJournalHeader) + (i * sizeof(FlashAttendanceEntry));
        STM32FlashDriver::readBuffer(addr, &entry, sizeof(entry));

        if (entry.marker == RECORD_MARKER_ERASED) {
            if (!foundBlank) {
                _nextWriteAddr = addr;
                foundBlank = true;
            }
        } else if (entry.marker == RECORD_MARKER_COMMITTED) {
            uint16_t calcCrc = calculateCRC16(reinterpret_cast<const uint8_t*>(&entry), sizeof(entry) - 2);
            if (calcCrc == entry.crc16) {
                _totalRecords++;
                if (entry.event_id > _highestRecordId) {
                    _highestRecordId = entry.event_id;
                }
            } else {
                LOG_WARN("STM32FlashJournal: Corrupted record detected at 0x%08lX (CRC mismatch)", addr);
            }
        }
    }

    if (!foundBlank && _totalRecords >= FLASH_MAX_ATTENDANCE_RECORDS) {
        _nextWriteAddr = FLASH_JOURNAL_BASE_ADDR + FLASH_JOURNAL_TOTAL_BYTES;
        LOG_WARN("STM32FlashJournal: Flash storage is completely FULL (853/853 records).");
    }

    LOG_INFO("STM32FlashJournal: Initialized. Stored=%u/853 records, HighestId=%lu, NextSlot=0x%08lX",
             (unsigned int)_totalRecords, _highestRecordId, _nextWriteAddr);
    return true;
}

bool STM32FlashJournal::logPunch(const String& uid, const String& name, uint64_t timestamp, uint32_t date,
                                uint8_t eventType, uint8_t sessionId, uint16_t flags, uint32_t& outEventId) {
    if (_totalRecords >= FLASH_MAX_ATTENDANCE_RECORDS ||
        _nextWriteAddr >= (FLASH_JOURNAL_BASE_ADDR + FLASH_JOURNAL_TOTAL_BYTES)) {
        LOG_ERROR("STM32FlashJournal: STORAGE FULL! Cannot mark attendance (Capacity: 853). Run CLEAR_LOGS or export.");
        return false;
    }

    // Verify slot is erased
    uint32_t checkWord;
    STM32FlashDriver::readBuffer(_nextWriteAddr, &checkWord, 4);
    if ((uint16_t)(checkWord & 0xFFFF) != RECORD_MARKER_ERASED) {
        LOG_ERROR("STM32FlashJournal: Target slot 0x%08lX is not erased.", _nextWriteAddr);
        return false;
    }

    String cleanUid = uid; cleanUid.trim(); cleanUid.toUpperCase();
    String cleanName = name; cleanName.trim();

    FlashAttendanceEntry entry;
    memset(&entry, 0, sizeof(entry));

    _highestRecordId++;
    outEventId = _highestRecordId;

    entry.marker = RECORD_MARKER_COMMITTED;
    entry.event_type = eventType;
    entry.session_id = sessionId;
    entry.event_id = outEventId;
    entry.timestamp = timestamp;
    entry.attendance_date = date;
    entry.flags = flags;

    strncpy(entry.uid, cleanUid.c_str(), sizeof(entry.uid) - 1);
    strncpy(entry.name_cache, cleanName.c_str(), sizeof(entry.name_cache) - 1);
    entry.crc16 = calculateCRC16(reinterpret_cast<const uint8_t*>(&entry), sizeof(entry) - 2);

    if (!STM32FlashDriver::writeBuffer(_nextWriteAddr, &entry, sizeof(entry))) {
        LOG_ERROR("STM32FlashJournal: Write failed at 0x%08lX", _nextWriteAddr);
        return false;
    }

    _totalRecords++;
    _nextWriteAddr += sizeof(FlashAttendanceEntry);
    LOG_INFO("STM32FlashJournal: Event #%lu logged [%s #%u] for %s (%u/853)",
             entry.event_id, (eventType == EVENT_TYPE_CHECK_IN) ? "IN" : "OUT",
             sessionId, cleanUid.c_str(), (unsigned int)_totalRecords);
    return true;
}

void STM32FlashJournal::forEachRecord(void (*callback)(const FlashAttendanceEntry& entry, void* context), void* context) {
    if (!callback) return;

    FlashAttendanceEntry entry;
    for (size_t i = 0; i < FLASH_MAX_ATTENDANCE_RECORDS; ++i) {
        uint32_t addr = FLASH_JOURNAL_BASE_ADDR + sizeof(FlashJournalHeader) + (i * sizeof(FlashAttendanceEntry));
        STM32FlashDriver::readBuffer(addr, &entry, sizeof(entry));

        if (entry.marker == RECORD_MARKER_COMMITTED) {
            uint16_t calcCrc = calculateCRC16(reinterpret_cast<const uint8_t*>(&entry), sizeof(entry) - 2);
            if (calcCrc == entry.crc16) {
                callback(entry, context);
            }
        }
    }
}

bool STM32FlashJournal::clearJournal() {
    uint32_t startPage = STM32FlashDriver::addressToPage(FLASH_JOURNAL_BASE_ADDR);
    if (!STM32FlashDriver::erasePages(startPage, FLASH_JOURNAL_TOTAL_PAGES)) {
        LOG_ERROR("STM32FlashJournal: Failed to erase journal pages in Zone C.");
        return false;
    }

    // Re-provision clean Journal Header
    FlashJournalHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = FLASH_JOURNAL_MAGIC;
    hdr.format_version = 1;
    hdr.total_capacity = FLASH_MAX_ATTENDANCE_RECORDS;
    hdr.crc16 = calculateCRC16(reinterpret_cast<const uint8_t*>(&hdr), sizeof(hdr) - 2);
    STM32FlashDriver::writeBuffer(FLASH_JOURNAL_BASE_ADDR, &hdr, sizeof(hdr));

    _totalRecords = 0;
    _highestRecordId = 0;
    _nextWriteAddr = FLASH_JOURNAL_BASE_ADDR + sizeof(FlashJournalHeader);
    LOG_INFO("STM32FlashJournal: Zone C journal completely cleared (853 slots available). Active state preserved.");
    return true;
}
