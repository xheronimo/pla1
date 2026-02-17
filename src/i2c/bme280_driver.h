#pragma once

#include <stdint.h>
#include "signal/signal_struct.h"
#include "i2c_chip_context.h"
#include "i2c_chip_registry.h"
#include "i2c_bus.h"

#define BME280_CACHE_MS  500

// API estándar
bool bme280Init(uint8_t addr, uint8_t options);
bool bme280ReadSignal(const Signal& s, float& out);
void bme280GetMetadata(ChipMetadata& meta);
void bme280Reset();
bool bme280Detect(uint8_t addr);