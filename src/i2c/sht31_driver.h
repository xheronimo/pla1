#pragma once

#include <stdint.h>
#include "signal/signal_struct.h"
#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_chip_registry.h"
#include "i2c/i2c_bus.h"

#define SHT31_CACHE_MS 500

// Canales:
// 0 → Temperature (°C)
// 1 → Humidity (%RH)

bool sht31Init(uint8_t addr, uint8_t options);
bool sht31ReadSignal(const Signal& s, float& out);
void sht31GetMetadata(ChipMetadata& meta);
void sht31Reset();
bool sht31Detect(uint8_t addr);