#pragma once

#include <stdint.h>
#include "signal/signal_struct.h"
#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_chip_registry.h"
#include "i2c/i2c_bus.h"

// Detect
bool pcfDetect8574(uint8_t addr);
bool pcfDetect8575(uint8_t addr);

// Init común
bool pcfInit(uint8_t addr, uint8_t options);

// Read / Write por Signal
bool pcfReadSignal(const Signal& s, float& out);
bool pcfWriteSignal(const Signal& s, float value);

// Metadata
void pcf8574GetMetadata(ChipMetadata& meta);
void pcf8575GetMetadata(ChipMetadata& meta);
