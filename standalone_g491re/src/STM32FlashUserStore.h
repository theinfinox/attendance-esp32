#pragma once
// ============================================================================
// STM32FlashUserStore.h — Ping-Pong Dual-Bank User Directory in Internal Flash
// ============================================================================

#include <Arduino.h>
#include <ArduinoJson.h>
#include "flash_storage.h"

class STM32FlashUserStore {
public:
    bool init();
    bool addUser(const String& uid, const String& name, const String& role = "User");
    bool deleteUser(const String& uid);
    String getUserName(const String& uid);
    String getAllUsersJson();
    uint16_t getUserCount() const { return _activeUserCount; }

private:
    uint32_t _activeBankAddr = FLASH_USER_BANK_A_ADDR;
    uint32_t _sequenceNumber = 0;
    uint16_t _activeUserCount = 0;

    bool validateBank(uint32_t bankAddr, uint32_t& seqOut, uint16_t& countOut);
    bool formatBank(uint32_t bankAddr, uint32_t seq);
    uint32_t getInactiveBankAddr() const {
        return (_activeBankAddr == FLASH_USER_BANK_A_ADDR) ? FLASH_USER_BANK_B_ADDR : FLASH_USER_BANK_A_ADDR;
    }
};

