#pragma once

#include <stdint.h>
#include "signal/signal_struct.h"
#include "i2c_bus.h"
#include "i2c_chip_context.h"
#include "i2c_chip_registry.h"

// ==============================
// CONFIG
// ==============================
#define PCF_MAX_CHIPS  8
#define PCF_CACHE_MS   50   // lectura rápida (digital)

// ==============================
// API ESTÁNDAR DRIVER
// ==============================

bool pcfInit(uint8_t addr, uint8_t options);
bool pcfReadSignal(const Signal& s, float& out);
bool pcfWriteSignal(const Signal& s, float value);
void pcf8574GetMetadata(ChipMetadata& meta);
void pcf8575GetMetadata(ChipMetadata& meta);

void pcfReset();
bool pcfDetect8575(uint8_t addr);
bool pcfDetect8574(uint8_t addr);