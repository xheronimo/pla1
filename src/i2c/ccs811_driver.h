#pragma once
#include "signal/signal_struct.h"
#include "i2c/i2c_chip_context.h"

#define CCS811_MAX_CHIPS   8
#define CCS811_CACHE_MS   1000   // 1s

// Canales CCS811
// 0 → eCO2 (ppm)
// 1 → TVOC (ppb)
bool ccs811Init(uint8_t addr, uint8_t driveMode = 1);
bool leerSignalCCS811(const Signal& s, float& out);
void ccs811Reset();
