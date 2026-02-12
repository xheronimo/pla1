#pragma once

#include <stdint.h>
#include "signal/signal_struct.h"
#include "i2c_chip_context.h"
#include "i2c_chip_registry.h"
#include "i2c_bus.h"

#define CCS811_CACHE_MS   1000

// Canales:
// 0 → eCO2 (ppm)
// 1 → TVOC (ppb)

bool ccs811Init(uint8_t addr, uint8_t options);
bool ccs811ReadSignal(const Signal& s, float& out);
void ccs811GetMetadata(ChipMetadata& meta);
void ccs811Reset();
bool ccs811Detect(uint8_t addr);