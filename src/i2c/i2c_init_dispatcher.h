#pragma once
#include <stdint.h>
#include "signal/signal_struct.h"

// Firma común de init
typedef bool (*I2CInitFn)(uint8_t addr, uint8_t options);

struct I2CChipInitEntry {
    I2CDevice chip;
    I2CInitFn initFn;
    uint32_t  warmupMs;
    uint32_t  retryMs;
};

// API
bool i2cInitChip(const Signal& s);
