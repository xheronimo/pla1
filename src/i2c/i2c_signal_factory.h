#pragma once

#include <vector>
#include "i2c/i2c_chip.h"

bool createSignalsForChip(I2CDevice type,
                          uint8_t addr,
                          const std::vector<uint8_t>& channels);
