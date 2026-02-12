#include "i2c_bus.h"
#include <Wire.h>
#include "i2c_chip_registry.h"


#define MAX_ADDR_PER_DEVICE 8

static SemaphoreHandle_t i2cMutex = nullptr;

// Tabla global real
static ChipContext chipCtx[(int)I2CDevice::MAX][MAX_ADDR_PER_DEVICE];

void i2cBusInit()
{
    if (!i2cMutex)
        i2cMutex = xSemaphoreCreateMutex();
}

SemaphoreHandle_t i2cGetMutex()
{
    return i2cMutex;
}

// ------------------------------------------------
// CONTEXT
// ------------------------------------------------
ChipContext* i2cGetContext(I2CDevice type, uint8_t addr)
{
    uint8_t idx = addr & 0x07;
    return &chipCtx[(int)type][idx];
}

// ------------------------------------------------
// LOW LEVEL
// ------------------------------------------------
bool i2cWrite8(uint8_t addr, uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

bool i2cWrite16(uint8_t addr, uint8_t reg, uint16_t val)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val >> 8);
    Wire.write(val & 0xFF);
    return Wire.endTransmission() == 0;
}

bool i2cRead8(uint8_t addr, uint8_t reg, uint8_t& out)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(addr, (uint8_t)1) != 1)
        return false;

    out = Wire.read();
    return true;
}

bool i2cRead16(uint8_t addr, uint8_t reg, uint16_t& out)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(addr, (uint8_t)2) != 2)
        return false;

    out = (Wire.read() << 8) | Wire.read();
    return true;
}

bool i2cReadBytes(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t len)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(addr, len) != len)
        return false;

    for (uint8_t i = 0; i < len; i++)
        buf[i] = Wire.read();

    return true;
}

bool i2cDetect(uint8_t addr)
{
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

bool i2cWriteRaw(uint8_t addr, uint8_t value)
{
    Wire.beginTransmission(addr);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool i2cReadRaw(uint8_t addr, uint8_t* buf, uint8_t len)
{
    if (Wire.requestFrom(addr, len) != len)
        return false;

    for (uint8_t i = 0; i < len; i++)
        buf[i] = Wire.read();

    return true;
}

bool i2cWriteBytes(uint8_t addr, uint8_t reg, const uint8_t* data, uint8_t len)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(data, len);
    return Wire.endTransmission() == 0;
}
void i2cBusReset()
{
    Wire.end();
    delay(50);
    Wire.begin();
}
