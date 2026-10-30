// ============================================================================
// STM32FlashUserStore.cpp — Ping-Pong Dual-Bank User Directory in Internal Flash
// ============================================================================

#include "STM32FlashUserStore.h"
#include "STM32FlashDriver.h"
#include "logger.h"

bool STM32FlashUserStore::validateBank(uint32_t bankAddr, uint32_t& seqOut, uint16_t& countOut) {
    FlashUserBankHeader header;
    STM32FlashDriver::readBuffer(bankAddr, &header, sizeof(header));

    if (header.magic != FLASH_USER_MAGIC) {
        return false;
    }
    if (header.user_count > 120) {
        return false;
    }

    // Verify CRC32 over all records in this bank
    size_t recordsSize = header.user_count * sizeof(FlashUserRecord);
    if (recordsSize > (FLASH_USER_BANK_SIZE - sizeof(FlashUserBankHeader))) {
        return false;
    }

    uint8_t buffer[64];
    uint32_t crc = 0xFFFFFFFF;
    for (uint16_t i = 0; i < header.user_count; ++i) {
        uint32_t recAddr = bankAddr + sizeof(FlashUserBankHeader) + (i * sizeof(FlashUserRecord));
        STM32FlashDriver::readBuffer(recAddr, buffer, sizeof(buffer));
        for (size_t b = 0; b < sizeof(buffer); ++b) {
            crc ^= buffer[b];
            for (uint8_t j = 0; j < 8; ++j) {
                if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
                else crc >>= 1;
            }
        }
    }
    crc = ~crc;

    if (crc != header.crc32) {
        LOG_WARN("STM32FlashUserStore: CRC mismatch at bank 0x%08lX (calc=0x%08lX, stored=0x%08lX)", bankAddr, crc, header.crc32);
        return false;
    }

    seqOut = header.sequence_number;
    countOut = header.user_count;
    return true;
}

bool STM32FlashUserStore::formatBank(uint32_t bankAddr, uint32_t seq) {
    uint32_t startPage = STM32FlashDriver::addressToPage(bankAddr);
    if (!STM32FlashDriver::erasePages(startPage, 4)) {
        return false;
    }

    FlashUserBankHeader header;
    memset(&header, 0, sizeof(header));
    header.magic = FLASH_USER_MAGIC;
    header.sequence_number = seq;
    header.user_count = 0;
    header.crc32 = 0;

    return STM32FlashDriver::writeBuffer(bankAddr, &header, sizeof(header));
}

bool STM32FlashUserStore::init() {
    uint32_t seqA = 0, seqB = 0;
    uint16_t countA = 0, countB = 0;
    bool validA = validateBank(FLASH_USER_BANK_A_ADDR, seqA, countA);
    bool validB = validateBank(FLASH_USER_BANK_B_ADDR, seqB, countB);

    if (validA && validB) {
        if (seqB > seqA) {
            _activeBankAddr = FLASH_USER_BANK_B_ADDR;
            _sequenceNumber = seqB;
            _activeUserCount = countB;
        } else {
            _activeBankAddr = FLASH_USER_BANK_A_ADDR;
            _sequenceNumber = seqA;
            _activeUserCount = countA;
        }
    } else if (validA) {
        _activeBankAddr = FLASH_USER_BANK_A_ADDR;
        _sequenceNumber = seqA;
        _activeUserCount = countA;
    } else if (validB) {
        _activeBankAddr = FLASH_USER_BANK_B_ADDR;
        _sequenceNumber = seqB;
        _activeUserCount = countB;
    } else {
        LOG_WARN("STM32FlashUserStore: Initializing blank User Directory in Flash");
        _sequenceNumber = 1;
        _activeUserCount = 0;
        _activeBankAddr = FLASH_USER_BANK_A_ADDR;
        formatBank(_activeBankAddr, _sequenceNumber);
    }

    LOG_INFO("STM32FlashUserStore: Active Bank=0x%08lX, Users=%u, Gen=%lu", _activeBankAddr, _activeUserCount, _sequenceNumber);
    return true;
}

String STM32FlashUserStore::getUserName(const String& uid) {
    if (_activeUserCount == 0) return "";
    String target = uid;
    target.trim();
    target.toUpperCase();

    FlashUserRecord rec;
    for (uint16_t i = 0; i < _activeUserCount; ++i) {
        uint32_t recAddr = _activeBankAddr + sizeof(FlashUserBankHeader) + (i * sizeof(FlashUserRecord));
        STM32FlashDriver::readBuffer(recAddr, &rec, sizeof(rec));
        if ((rec.flags & 0x0001) && strcmp(rec.uid, target.c_str()) == 0) {
            return String(rec.name);
        }
    }
    return "";
}

String STM32FlashUserStore::getAllUsersJson() {
    JsonDocument doc;
    FlashUserRecord rec;
    for (uint16_t i = 0; i < _activeUserCount; ++i) {
        uint32_t recAddr = _activeBankAddr + sizeof(FlashUserBankHeader) + (i * sizeof(FlashUserRecord));
        STM32FlashDriver::readBuffer(recAddr, &rec, sizeof(rec));
        if (rec.flags & 0x0001) {
            doc[rec.uid] = rec.name;
        }
    }
    String out;
    serializeJson(doc, out);
    return out;
}

bool STM32FlashUserStore::addUser(const String& uid, const String& name, const String& role) {
    uint32_t targetBank = getInactiveBankAddr();
    uint32_t startPage = STM32FlashDriver::addressToPage(targetBank);
    if (!STM32FlashDriver::erasePages(startPage, 4)) {
        return false;
    }

    String cleanUid = uid; cleanUid.trim(); cleanUid.toUpperCase();
    String cleanName = name; cleanName.trim();
    String cleanRole = role; cleanRole.trim();

    FlashUserRecord tempRec;
    uint16_t newCount = 0;
    uint32_t targetRecAddr = targetBank + sizeof(FlashUserBankHeader);

    // Copy existing active records (except if matching UID, which gets replaced)
    for (uint16_t i = 0; i < _activeUserCount; ++i) {
        uint32_t srcAddr = _activeBankAddr + sizeof(FlashUserBankHeader) + (i * sizeof(FlashUserRecord));
        STM32FlashDriver::readBuffer(srcAddr, &tempRec, sizeof(tempRec));
        if (strcmp(tempRec.uid, cleanUid.c_str()) == 0) {
            continue; // Will be replaced by new version
        }
        STM32FlashDriver::writeBuffer(targetRecAddr, &tempRec, sizeof(tempRec));
        targetRecAddr += sizeof(FlashUserRecord);
        newCount++;
    }

    // Append new record
    memset(&tempRec, 0, sizeof(tempRec));
    strncpy(tempRec.uid, cleanUid.c_str(), sizeof(tempRec.uid) - 1);
    strncpy(tempRec.name, cleanName.c_str(), sizeof(tempRec.name) - 1);
    strncpy(tempRec.role, cleanRole.c_str(), sizeof(tempRec.role) - 1);
    tempRec.flags = 0x0001;
    tempRec.crc16 = calculateCRC16(reinterpret_cast<const uint8_t*>(&tempRec), sizeof(tempRec) - 2);

    STM32FlashDriver::writeBuffer(targetRecAddr, &tempRec, sizeof(tempRec));
    newCount++;

    // Calculate CRC32 of all written records
    uint8_t buffer[64];
    uint32_t crc = 0xFFFFFFFF;
    for (uint16_t i = 0; i < newCount; ++i) {
        uint32_t rAddr = targetBank + sizeof(FlashUserBankHeader) + (i * sizeof(FlashUserRecord));
        STM32FlashDriver::readBuffer(rAddr, buffer, sizeof(buffer));
        for (size_t b = 0; b < sizeof(buffer); ++b) {
            crc ^= buffer[b];
            for (uint8_t j = 0; j < 8; ++j) {
                if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
                else crc >>= 1;
            }
        }
    }
    crc = ~crc;

    // Write Header
    FlashUserBankHeader header;
    header.magic = FLASH_USER_MAGIC;
    header.sequence_number = _sequenceNumber + 1;
    header.user_count = newCount;
    header.reserved = 0;
    header.crc32 = crc;

    if (!STM32FlashDriver::writeBuffer(targetBank, &header, sizeof(header))) {
        return false;
    }

    // Atomic switch
    _activeBankAddr = targetBank;
    _sequenceNumber++;
    _activeUserCount = newCount;
    LOG_INFO("STM32FlashUserStore: User %s added. Total=%u, Gen=%lu", cleanUid.c_str(), _activeUserCount, _sequenceNumber);
    return true;
}

bool STM32FlashUserStore::deleteUser(const String& uid) {
    uint32_t targetBank = getInactiveBankAddr();
    uint32_t startPage = STM32FlashDriver::addressToPage(targetBank);
    if (!STM32FlashDriver::erasePages(startPage, 4)) {
        return false;
    }

    String cleanUid = uid; cleanUid.trim(); cleanUid.toUpperCase();
    FlashUserRecord tempRec;
    uint16_t newCount = 0;
    uint32_t targetRecAddr = targetBank + sizeof(FlashUserBankHeader);
    bool found = false;

    for (uint16_t i = 0; i < _activeUserCount; ++i) {
        uint32_t srcAddr = _activeBankAddr + sizeof(FlashUserBankHeader) + (i * sizeof(FlashUserRecord));
        STM32FlashDriver::readBuffer(srcAddr, &tempRec, sizeof(tempRec));
        if (strcmp(tempRec.uid, cleanUid.c_str()) == 0) {
            found = true;
            continue; // Skip writing this user
        }
        STM32FlashDriver::writeBuffer(targetRecAddr, &tempRec, sizeof(tempRec));
        targetRecAddr += sizeof(FlashUserRecord);
        newCount++;
    }

    if (!found) {
        return false;
    }

    // Calculate CRC32 of all written records
    uint8_t buffer[64];
    uint32_t crc = 0xFFFFFFFF;
    for (uint16_t i = 0; i < newCount; ++i) {
        uint32_t rAddr = targetBank + sizeof(FlashUserBankHeader) + (i * sizeof(FlashUserRecord));
        STM32FlashDriver::readBuffer(rAddr, buffer, sizeof(buffer));
        for (size_t b = 0; b < sizeof(buffer); ++b) {
            crc ^= buffer[b];
            for (uint8_t j = 0; j < 8; ++j) {
                if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
                else crc >>= 1;
            }
        }
    }
    crc = ~crc;

    FlashUserBankHeader header;
    header.magic = FLASH_USER_MAGIC;
    header.sequence_number = _sequenceNumber + 1;
    header.user_count = newCount;
    header.reserved = 0;
    header.crc32 = crc;

    if (!STM32FlashDriver::writeBuffer(targetBank, &header, sizeof(header))) {
        return false;
    }

    _activeBankAddr = targetBank;
    _sequenceNumber++;
    _activeUserCount = newCount;
    LOG_INFO("STM32FlashUserStore: User %s deleted. Total=%u, Gen=%lu", cleanUid.c_str(), _activeUserCount, _sequenceNumber);
    return true;
}

