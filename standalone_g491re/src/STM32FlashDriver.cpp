// ============================================================================
// STM32FlashDriver.cpp — Low-Level HAL Flash Interface for STM32G491RE
// ============================================================================

#include "STM32FlashDriver.h"
#include "logger.h"

#if defined(ARDUINO_ARCH_STM32)
#include "stm32g4xx_hal.h"

bool STM32FlashDriver::init() {
    return true;
}

bool STM32FlashDriver::erasePage(uint32_t pageIndex) {
    return erasePages(pageIndex, 1);
}

bool STM32FlashDriver::erasePages(uint32_t startPage, uint32_t pageCount) {
    if (startPage < 224 || (startPage + pageCount) > 256) {
        LOG_ERROR("STM32Flash: Refusing erase outside storage partition (Pages 224..255). Target: %lu", startPage);
        return false;
    }

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    FLASH_EraseInitTypeDef eraseInit;
    eraseInit.TypeErase   = FLASH_TYPEERASE_PAGES;
    eraseInit.Banks       = FLASH_BANK_1;
    eraseInit.Page        = startPage;
    eraseInit.NbPages     = pageCount;

    uint32_t pageError = 0;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&eraseInit, &pageError);
    HAL_FLASH_Lock();

    if (status != HAL_OK) {
        LOG_ERROR("STM32Flash: Erase failed at page %lu, err=%lu", pageError, (uint32_t)status);
        return false;
    }
    return true;
}

bool STM32FlashDriver::writeDoubleWord(uint32_t address, uint64_t data) {
    if (address < FLASH_STORAGE_BASE_ADDR || (address + 8) > (FLASH_STORAGE_END_ADDR + 1)) {
        LOG_ERROR("STM32Flash: Write out of bounds: 0x%08lX", address);
        return false;
    }
    if ((address % 8) != 0) {
        LOG_ERROR("STM32Flash: Address not 64-bit aligned: 0x%08lX", address);
        return false;
    }

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data);
    HAL_FLASH_Lock();

    if (status != HAL_OK) {
        LOG_ERROR("STM32Flash: Program DWORD failed at 0x%08lX (status=%d)", address, (int)status);
        return false;
    }
    return true;
}

bool STM32FlashDriver::writeBuffer(uint32_t address, const void* buffer, size_t sizeBytes) {
    if ((sizeBytes % 8) != 0) {
        LOG_ERROR("STM32Flash: Buffer size must be multiple of 8: %u", sizeBytes);
        return false;
    }

    const uint64_t* src = reinterpret_cast<const uint64_t*>(buffer);
    size_t dwordCount = sizeBytes / 8;

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    for (size_t i = 0; i < dwordCount; ++i) {
        uint32_t targetAddr = address + (i * 8);
        if (targetAddr < FLASH_STORAGE_BASE_ADDR || (targetAddr + 8) > (FLASH_STORAGE_END_ADDR + 1)) {
            HAL_FLASH_Lock();
            return false;
        }
        HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, targetAddr, src[i]);
        if (status != HAL_OK) {
            HAL_FLASH_Lock();
            LOG_ERROR("STM32Flash: Block write failed at 0x%08lX", targetAddr);
            return false;
        }
    }

    HAL_FLASH_Lock();
    return true;
}

void STM32FlashDriver::readBuffer(uint32_t address, void* buffer, size_t sizeBytes) {
    const uint8_t* flashPtr = reinterpret_cast<const uint8_t*>(address);
    uint8_t* dst = reinterpret_cast<uint8_t*>(buffer);
    for (size_t i = 0; i < sizeBytes; ++i) {
        dst[i] = flashPtr[i];
    }
}

#endif // ARDUINO_ARCH_STM32

