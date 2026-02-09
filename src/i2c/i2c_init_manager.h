
#pragma once
#include "i2c_chip_registry.h"
#include "i2c_scan.h"


void i2cInitConfiguredChips(const Signal* signals, size_t count); 
void i2cInitFromSignals();
