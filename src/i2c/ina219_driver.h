#pragma once
#include "signal/signal_struct.h"
#include "i2c_chip_context.h"

#define INA219_MAX_CHIPS 8
#define INA219_CACHE_MS  500

bool leerSignalINA219(const Signal& s, float& out, uint8_t mode =0);
void ina219Reset();
