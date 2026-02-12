#pragma once

#include <stdint.h>
#include "signal/signal_struct.h"
#include "i2c_chip_context.h"
#include "i2c_chip_registry.h"
#include "i2c_bus.h"

#define BH1750_CACHE_MS 500

// Canal único:
// 0 → Lux

bool bh1750Init(uint8_t addr, uint8_t options);
bool bh1750ReadSignal(const Signal& s, float& out);
void bh1750GetMetadata(ChipMetadata& meta);
void bh1750Reset();

typedef bool (*ChipDetectFn)(uint8_t addr);
typedef bool (*ChipInitFn)(uint8_t addr, uint8_t options);
typedef bool (*ChipReadFn)(const Signal& s, float& out);
typedef void (*ChipMetadataFn)(ChipMetadata& meta);

bool bh1750Detect(uint8_t addr);