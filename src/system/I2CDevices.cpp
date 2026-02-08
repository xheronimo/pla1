#include "I2CDevices.h"
#include <Wire.h>

bool leerI2CDigital(uint8_t addr, uint8_t bit, bool& out)
{
    Wire.beginTransmission(addr);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0)
        return false;

    Wire.requestFrom(addr, (uint8_t)1);
    if (!Wire.available())
        return false;

    uint8_t v = Wire.read();
    out = (v >> bit) & 0x01;
    return true;
}

bool leerI2CAnalog(uint8_t addr, uint8_t channel, float& out)
{
    Wire.beginTransmission(addr);
    Wire.write(channel);
    if (Wire.endTransmission() != 0)
        return false;

    Wire.requestFrom(addr, (uint8_t)2);
    if (Wire.available() < 2)
        return false;

    uint16_t v = (Wire.read() << 8) | Wire.read();
    out = (float)v;
    return true;
}
