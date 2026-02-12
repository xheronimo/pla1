#pragma once

#include <stdint.h>
#include "signal/signal_struct.h"
#include "i2c_chip_context.h"
#include "i2c_chip_registry.h"
#include "i2c_bus.h"

#define ENS160_CACHE_MS 1000

// Canales:
// 0 → AQI
// 1 → TVOC (ppb)
// 2 → eCO2 (ppm)

bool ens160Init(uint8_t addr, uint8_t options);
bool ens160ReadSignal(const Signal& s, float& out);
void ens160GetMetadata(ChipMetadata& meta);
void ens160Reset();

// Opcional: compensación ambiental
bool ens160SetEnvironmentalData(uint8_t addr, float temp, float hum);

bool ens160Detect(uint8_t addr);