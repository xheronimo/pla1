#pragma once
#include <vector>
#include "i2c_chip_registry.h"

struct I2CDetectedChip
{
    I2CDevice type;
    uint8_t address;
};

std::vector<I2CDetectedChip> i2cAutoDetect();