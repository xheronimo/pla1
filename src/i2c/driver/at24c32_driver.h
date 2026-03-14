#pragma once
#include "signal/signal_struct.h"
#include "i2c/i2c_chip_context.h"

bool at24c32Init(uint8_t addr, uint8_t options);
bool at24c32Detect(uint8_t addr);
bool at24c32ReadSignal(const Signal& s, float& out);
void at24c32GetMetadata(ChipMetadata& meta);