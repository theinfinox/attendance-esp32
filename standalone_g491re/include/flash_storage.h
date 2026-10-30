#pragma once
// ============================================================================
// flash_storage.h — STM32G491RE Internal Flash Storage Memory Layout & Structs
// Partition: 64 KB Total (Pages 224 to 255: 0x08070000 to 0x0807FFFF)
// Zone A: Registered Users Directory (16 KB, Pages 224..231)
// Zone B: Active State Store (8 KB, Pages 232..235) — PROTECTED FROM CLEAR_LOGS
// Zone C: Attendance Event Journal (40 KB, Pages 236..255) — ERASED BY CLEAR_LOGS
// ============================================================================

#include <Arduino.h>

// --- Physical Layout on STM32G491RE (512 KB Total Flash) ---
// Page Size: 2048 bytes (2 KB)
constexpr uint32_t STM32_FLASH_PAGE_SIZE        = 2048;

constexpr uint32_t FLASH_STORAGE_BASE_ADDR     = 0x08070000;
constexpr uint32_t FLASH_STORAGE_END_ADDR      = 0x0807FFFF;
constexpr uint32_t FLASH_STORAGE_TOTAL_PAGES   = 32;

// ============================================================================
// Zone A: Registered Students Directory (16 KB / 8 Pages: 224..231)
// Dual-Bank / Ping-Pong Architecture for atomic power-loss safety
// ============================================================================
constexpr uint32_t FLASH_USER_BANK_SIZE        = 8192; // 8 KB (4 pages) per bank
constexpr uint32_t FLASH_USER_BANK_A_ADDR      = 0x08070000; // Pages 224..227
constexpr uint32_t FLASH_USER_BANK_B_ADDR      = 0x08072000; // Pages 228..231
constexpr uint32_t FLASH_USER_MAGIC            = 0x55534552; // "USER"

struct __attribute__((packed, aligned(8))) FlashUserRecord {
    char     uid[16];      // Null-terminated hex string (e.g. "04A1B2C3")
    char     name[36];     // Null-terminated UTF-8 student name
    char     role[8];      // "User", "Admin", etc.
    uint16_t flags;        // 0x0001 = Active, 0x0002 = Deleted
    uint16_t crc16;        // Record-level checksum
}; // Exactly 64 bytes (8 × 64-bit DWORDs)

struct __attribute__((packed, aligned(8))) FlashUserBankHeader {
    uint32_t magic;           // FLASH_USER_MAGIC
    uint32_t sequence_number; // Monotonically increasing generation number
    uint16_t user_count;      // Active records in this bank
    uint16_t reserved;
    uint32_t crc32;           // CRC32 of all subsequent record bytes
}; // Exactly 16 bytes (2 × 64-bit DWORDs)

constexpr size_t FLASH_MAX_USERS = (FLASH_USER_BANK_SIZE - sizeof(FlashUserBankHeader)) / sizeof(FlashUserRecord); // 127 (capped to 126)

// ============================================================================
// Zone B: Active Attendance State Store (8 KB / 4 Pages: 232..235)
// Tracks current IN/OUT state for active attendance day.
// NEVER erased by CLEAR_LOGS. Ping-Pong compacted between Bank 0 and Bank 1.
// ============================================================================
constexpr uint32_t FLASH_STATE_BASE_ADDR       = 0x08074000; // Page 232
constexpr uint32_t FLASH_STATE_TOTAL_PAGES     = 4;
constexpr uint32_t FLASH_STATE_BANK_SIZE       = 4096; // 4 KB (2 pages) per bank
constexpr uint32_t FLASH_STATE_BANK_0_ADDR     = 0x08074000; // Pages 232..233
constexpr uint32_t FLASH_STATE_BANK_1_ADDR     = 0x08075000; // Pages 234..235
constexpr uint32_t FLASH_STATE_MAGIC           = 0x53544154; // "STAT"

constexpr uint8_t  RECORD_STATE_CHECKED_OUT    = 0;
constexpr uint8_t  RECORD_STATE_CHECKED_IN     = 1;

struct __attribute__((packed, aligned(8))) FlashStateBankHeader {
    uint32_t magic;           // FLASH_STATE_MAGIC ("STAT")
    uint32_t generation;      // Monotonically increasing compaction generation counter
    uint32_t active_date;     // Current attendance calendar date (YYYYMMDD)
    uint16_t state_count;     // Number of transition records committed in this bank
    uint16_t crc16;           // Header checksum
}; // Exactly 16 bytes (2 × 64-bit DWORDs)

struct __attribute__((packed, aligned(8))) FlashStateTransitionRecord {
    // DWORD 0: Markers, State, Session & Event Reference
    uint16_t marker;          // 0x55AA (COMMITTED) or 0xFFFF (BLANK)
    uint8_t  state;           // 1 = CHECKED_IN, 2 = CHECKED_OUT
    uint8_t  session_id;      // Session counter today (1, 2, 3...)
    uint32_t event_id;        // Unique event index corresponding to Zone C entry
    
    // DWORD 1: Timestamps & Calendar Date
    uint32_t timestamp;       // UNIX epoch seconds of transition
    uint32_t date;            // YYYYMMDD
    
    // DWORD 2: Primary Card UID
    char     uid[8];          // 7 hex chars + null (e.g. "04A1B2C3")
    
    // DWORD 3: Extended UID, Flags & Checksum
    char     uid_ext[4];      // Extended hex chars for 7-byte MIFARE UIDs
    uint16_t flags;           // 0x0001 = Normal, 0x0002 = TimeUnsynced
    uint16_t crc16;           // CRC16 of DWORDs 0 to 2 + first 6 bytes of DWORD 3
}; // Exactly 32 bytes (4 × 64-bit DWORDs)

constexpr size_t FLASH_MAX_STATE_TRANSITIONS = (FLASH_STATE_BANK_SIZE - sizeof(FlashStateBankHeader)) / sizeof(FlashStateTransitionRecord); // 127

// ============================================================================
// Zone C: Attendance Event Journal (40 KB / 20 Pages: 236..255)
// Append-only historical log for PWA export. Erased when CLEAR_LOGS runs.
// ============================================================================
constexpr uint32_t FLASH_JOURNAL_BASE_ADDR     = 0x08076000; // Page 236
constexpr uint32_t FLASH_JOURNAL_TOTAL_PAGES   = 20;
constexpr uint32_t FLASH_JOURNAL_TOTAL_BYTES   = FLASH_JOURNAL_TOTAL_PAGES * STM32_FLASH_PAGE_SIZE; // 40,960 bytes
constexpr uint32_t FLASH_JOURNAL_MAGIC         = 0x4A4F5552; // "JOUR"

constexpr uint16_t RECORD_MARKER_ERASED        = 0xFFFF; // Unwritten Flash slot
constexpr uint16_t RECORD_MARKER_COMMITTED     = 0x55AA; // Valid local punch (unsynced)
constexpr uint16_t RECORD_MARKER_SYNCED        = 0x0000; // Reconciled / committed to host

constexpr uint8_t  EVENT_TYPE_CHECK_IN         = 1;
constexpr uint8_t  EVENT_TYPE_CHECK_OUT        = 2;

constexpr uint16_t RECORD_FLAG_NORMAL          = 0x0001;
constexpr uint16_t RECORD_FLAG_UNSYNCED        = 0x0002;

struct __attribute__((packed, aligned(8))) FlashJournalHeader {
    uint32_t magic;           // FLASH_JOURNAL_MAGIC ("JOUR")
    uint32_t format_version;  // 0x00000001
    uint32_t total_capacity;  // 853
    uint16_t reserved;
    uint16_t crc16;           // Header checksum
}; // Exactly 16 bytes (2 × 64-bit DWORDs)

struct __attribute__((packed, aligned(8))) FlashAttendanceEntry {
    // DWORD 0: Marker, Event Type, Session, Event ID
    uint16_t marker;          // 0x55AA = COMMITTED, 0x0000 = SYNCED, 0xFFFF = BLANK
    uint8_t  event_type;      // 1 = CHECK_IN, 2 = CHECK_OUT
    uint8_t  session_id;      // Session index for student today (1, 2...)
    uint32_t event_id;        // Monotonically increasing unique event index (1, 2, 3...)
    
    // DWORD 1: Full UNIX Timestamp
    uint64_t timestamp;       // UNIX epoch seconds (64-bit)
    
    // DWORD 2 & 3: Full Hex Card UID
    char     uid[16];         // Null-terminated hex string (up to 15 hex chars + null)
    
    // DWORD 4: Calendar Date & Integrity Flags
    uint32_t attendance_date; // YYYYMMDD (e.g. 20261030)
    uint16_t flags;           // 0x0001 = Normal, 0x0002 = TimeUnsynced
    uint16_t reserved;        // Alignment padding
    
    // DWORD 5: Student Name Cache & Checksum
    char     name_cache[6];   // 5 ASCII chars + null terminator
    uint16_t crc16;           // CRC16 of preceding 46 bytes
}; // Exactly 48 bytes (6 × 64-bit DWORDs)

constexpr size_t FLASH_MAX_ATTENDANCE_RECORDS  = (FLASH_JOURNAL_TOTAL_BYTES - sizeof(FlashJournalHeader)) / sizeof(FlashAttendanceEntry); // Exactly 853 records

// --- Checksum Helpers ---
inline uint16_t calculateCRC16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

inline uint32_t calculateCRC32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return ~crc;
}
