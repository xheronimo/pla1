#pragma once

#include <stdint.h>
#include "signal/signal_struct.h"
#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_chip_registry.h"
#include "i2c/i2c_bus.h"

#define INA219_CACHE_MS 300

// Canales:
// 0 → Voltage (V)
// 1 → Current (A)
// 2 → Power (W)

bool ina219Init(uint8_t addr, uint8_t options);
bool ina219ReadSignal(const Signal& s, float& out);
void ina219GetMetadata(ChipMetadata& meta);
void ina219Reset();
bool ina219Detect(uint8_t addr);