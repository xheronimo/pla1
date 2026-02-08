#include "i2c/pcf8575_driver.h"

// -----------------------------
// CACHE
// -----------------------------
static Pcf8575Cache pcf8575Cache[PCF8575_MAX_CHIPS];

// -----------------------------
void pcf8575ResetCaches()
{
    for (uint8_t i = 0; i < PCF8575_MAX_CHIPS; i++)
        pcf8575Cache[i] = {};
}

// -----------------------------
// LECTURA I2C 16 bits
// -----------------------------
static bool readPCF8575(uint8_t addr, uint16_t& out)
{
    Wire.requestFrom(addr, (uint8_t)2);
    if (Wire.available() < 2)
        return false;

    uint8_t lo = Wire.read();
    uint8_t hi = Wire.read();

    out = (uint16_t)hi << 8 | lo;
    return true;
}

// -----------------------------
// ACTUALIZAR CACHE
// -----------------------------
static bool updateCache(uint8_t addr, Pcf8575Cache& c)
{
    uint16_t data;

    if (!readPCF8575(addr, data))
    {
        c.errorCount++;
        c.valid = false;
        return false;
    }

    c.value      = data;
    c.lastReadMs = millis();
    c.valid      = true;
    c.errorCount = 0;
    return true;
}

// -----------------------------
// API PRINCIPAL
// -----------------------------
bool leerSignalPCF8575(const Signal& s, float& out)
{
    // channel = 0..15
    if (s.channel > 15)
        return false;

    uint8_t idx = s.address & 0x07;
    Pcf8575Cache& c = pcf8575Cache[idx];

    if (!c.valid || millis() - c.lastReadMs > PCF8575_CACHE_MS)
    {
        if (!updateCache(s.address, c))
            return false;
    }

    bool level = (c.value & (1 << s.channel)) != 0;
    out = level ? 1.0f : 0.0f;
    return true;
}
