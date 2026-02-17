#pragma once
#include <stdint.h>
#include "i2c_chip.h"
#include "i2c_chip_registry.h"

#define I2C_MAX_TYPES 16
#define I2C_MAX_CHIPS_PER_TYPE 8

enum class ChipState : uint8_t
{
    STATE_UNINITIALIZED = 0,
    STATE_INITIALIZING,
    STATE_WARMUP,
    STATE_READY,
    STATE_ERROR,
    STATE_TIMEOUT,
    STATE_LOCKED   // ⛔ reemplaza a DISABLED
};

struct ChipContext
{
    bool inUse = false;
    I2CDevice type = I2CDevice::NONE;
    uint8_t address;

    ChipState state = ChipState::STATE_UNINITIALIZED;

    uint32_t initTs = 0;
    uint32_t lastReadTs = 0;

    uint32_t warmupMs = 0;
    uint32_t retryMs = 2000;

    uint8_t errorCount = 0;
    uint8_t consecutiveErrors = 0;

    bool busy = false;
    bool fatal = false;
    
    uint32_t totalReads = 0;
    uint32_t totalErrors = 0;
    bool disabled = false;

    uint32_t lastAttemptTs = 0;
    uint32_t lastOkTs = 0;
    I2CChipDriver* driver = nullptr;
    uint8_t options;   
    bool optionsSet = false;

    uint16_t shadow = 0xFFFF;   // último valor escrito
    bool shadowValid = false;

      // 🔐 NUEVO – Watchdog I2C
    uint8_t  i2cFailCount;
    uint32_t lastI2cFailTs;
    bool    allocated = false;
};

// 🔥 Forward declaration limpia
struct Signal;

ChipContext* i2cGetChipContext(I2CDevice type, uint8_t addr);

float i2cChipHealth(const ChipContext* ctx);