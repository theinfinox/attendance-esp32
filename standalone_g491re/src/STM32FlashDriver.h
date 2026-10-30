#pragma once
// ============================================================================
// STM32FlashDriver.h — Low-Level HAL Flash Interface for STM32G491RE
// ============================================================================

#include <Arduino.h>
#include "flash_storage.h"

class STM32FlashDriver {
public:
    static bool init();
    static bool erasePage(uint32_t pageIndex);
    static bool erasePages(uint32_t startPage, uint32_t pageCount);
    static bool writeDoubleWord(uint32_t address, uint64_t data);
    static bool writeBuffer(uint32_t address, const void* buffer, size_t sizeBytes);
    static void readBuffer(uint32_t address, void* buffer, size_t sizeBytes);
    
    // Address to page index calculation (0x08000000 -> Page 0)
    static uint32_t addressToPage(uint32_t address) {
        return (address - 0x08000000) / STM32_FLASH_PAGE_SIZE;
    }
};

