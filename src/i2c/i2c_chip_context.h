#pragma once
#include <stdint.h>
#include "i2c_chip.h"
#define I2C_MAX_TYPES 16
#define I2C_MAX_CHIPS_PER_TYPE 8
enum class ChipState : uint8_t {
    UNINITIALIZED,
    INITIALIZING,
    WARMUP,
    READY,
    ERROR
};

struct ChipContext
{
    I2CDevice type = I2CDevice::NONE;

    ChipState state = ChipState::UNINITIALIZED;

    uint32_t initTs = 0;
    uint32_t lastReadTs = 0;

    uint32_t warmupMs = 0;
    uint32_t retryMs = 0;

    uint8_t errorCount = 0;
    uint8_t consecutiveErrors = 0;

    bool busy = false;
    bool fatal = false;
};

// 🔥 Forward declaration limpia
struct Signal;

ChipContext* i2cGetChipContext(I2CDevice type, uint8_t addr);
