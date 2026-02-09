#include <Wire.h>
#include "i2c_chip_context.h"

bool i2cDetect(uint8_t addr)
{
    Wire.beginTransmission(addr);
    return (Wire.endTransmission() == 0);
}
