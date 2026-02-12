#pragma once
#include "i2c_bus.h"
#include <vector>


void registerI2CCreateAPI();
bool createSignalsForChip(I2CDevice type,
                          uint8_t addr,
                          const std::vector<uint8_t>& channels);