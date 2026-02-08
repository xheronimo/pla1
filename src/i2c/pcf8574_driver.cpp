#include "i2c/pcf8574_driver.h"

// --------------------------------------------------
// CACHES SEPARADAS
// --------------------------------------------------
static PcfCache pcfUserCache[PCF_MAX_CHIPS];
static PcfCache pcfSystemCache[PCF_MAX_CHIPS];

// --------------------------------------------------
void pcf8574ResetCaches()
{
    for (uint8_t i = 0; i < PCF_MAX_CHIPS; i++)
    {
        pcfUserCache[i]   = {};
        pcfSystemCache[i] = {};
    }
}

// --------------------------------------------------
// LECTURA BAJO NIVEL (1 BYTE)
// --------------------------------------------------
static bool readPCFByte(uint8_t addr, uint8_t& out)
{
    Wire.requestFrom(addr, (uint8_t)1);
    if (Wire.available() < 1)
        return false;

    out = Wire.read();
    return true;
}

// --------------------------------------------------
// ACTUALIZAR CACHE
// --------------------------------------------------
static bool updateCache(uint8_t addr, PcfCache& c)
{
    uint8_t data;

    if (!readPCFByte(addr, data))
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

// --------------------------------------------------
// LECTURA COMÚN (con cache)
// --------------------------------------------------
static bool leerPCF(
    const Signal& s,
    PcfCache* cacheArray,
    float& out
)
{
    uint8_t addr = s.address;
    uint8_t bit  = s.channel;

    if (bit > 7)
        return false;

    uint8_t idx = addr & 0x07;   // 0x20..0x27 → 0..7
    PcfCache& c = cacheArray[idx];

    if (!c.valid || millis() - c.lastReadMs > PCF_CACHE_MS)
    {
        if (!updateCache(addr, c))
            return false;
    }

    bool level = (c.value & (1 << bit)) != 0;
    out = level ? 1.0f : 0.0f;
    return true;
}

// --------------------------------------------------
// API PÚBLICA
// --------------------------------------------------
bool leerSignalPCF_User(const Signal& s, float& out)
{
    return leerPCF(s, pcfUserCache, out);
}

bool leerSignalPCF_System(const Signal& s, float& out)
{
    return leerPCF(s, pcfSystemCache, out);
}
