#include "i2c/pcf_driver.h"
#include <Wire.h>

// ------------------------------
// CACHE
// ------------------------------
static PcfCache pcfCache[PCF_MAX_CHIPS];

// ------------------------------
// LOW LEVEL
// ------------------------------
static bool readPCF(
    uint8_t addr,
    I2CDevice chip,
    uint16_t& out
)
{
    if (chip == I2CDevice::PCF8574)
    {
        Wire.requestFrom(addr, (uint8_t)1);
        if (Wire.available() < 1)
            return false;

        out = Wire.read();
        return true;
    }

    if (chip == I2CDevice::PCF8575)
    {
        Wire.requestFrom(addr, (uint8_t)2);
        if (Wire.available() < 2)
            return false;

        uint8_t lo = Wire.read();
        uint8_t hi = Wire.read();
        out = (hi << 8) | lo;
        return true;
    }

    return false;
}

// ------------------------------
// API PRINCIPAL
// ------------------------------
bool leerSignalPCF(const Signal& s, float& out)
{
    uint8_t idx = s.address & 0x07;
    if (idx >= PCF_MAX_CHIPS)
        return false;

    PcfCache& c = pcfCache[idx];
    uint32_t now = millis();

    if (!c.valid || now - c.lastReadMs > PCF_CACHE_MS)
    {
        if (!readPCF(s.address, s.chip, c.value))
        {
            c.errorCount++;
            c.valid = false;
            return false;
        }

        c.valid = true;
        c.lastReadMs = now;
        c.errorCount = 0;
    }

    // ------------------------------
    // EXTRAER BIT
    // ------------------------------
    uint8_t bit = s.channel;

    if (s.chip == I2CDevice::PCF8574 && bit > 7)
        return false;

    if (s.chip == I2CDevice::PCF8575 && bit > 15)
        return false;

    bool level = (c.value & (1 << bit)) != 0;
    out = level ? 1.0f : 0.0f;
    return true;
}

// ------------------------------
void pcfResetCaches()
{
    for (uint8_t i = 0; i < PCF_MAX_CHIPS; i++)
        pcfCache[i] = {};
}
