#pragma once
#include "signal/signal_struct.h"
#include "i2c_chip_context.h"

#define SHT31_MAX_CHIPS 8
#define SHT31_CACHE_MS  500

bool sht31Init(uint8_t addr, uint8_t mode = 0);
bool leerSignalSHT31(const Signal& s, float& out);
void sht31Reset();
//