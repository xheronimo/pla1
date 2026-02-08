#include "ccs811_driver.h"

// -----------------------------
#define CCS811_REG_STATUS     0x00
#define CCS811_REG_MEAS_MODE  0x01
#define CCS811_REG_ALG_RESULT 0x02
#define CCS811_REG_HW_ID      0x20

#define CCS811_HW_ID          0x81

#define CCS811_DRIVE_MODE_1S  0x10

static CCS811Cache cache[CCS811_MAX_CHIPS];

// -----------------------------
void ccs811ResetCaches()
{
    for (uint8_t i = 0; i < CCS811_MAX_CHIPS; i++)
        cache[i] = {};
}

// -----------------------------
static bool i2cRead(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t len)
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

// -----------------------------
static bool i2cWrite(uint8_t addr, uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

// -----------------------------
static bool ccs811Init(uint8_t addr, CCS811Cache& c)
{
    uint8_t id;
    if (!i2cRead(addr, CCS811_REG_HW_ID, &id, 1))
        return false;

    if (id != CCS811_HW_ID)
        return false;

    // Modo medición cada 1s
    if (!i2cWrite(addr, CCS811_REG_MEAS_MODE, CCS811_DRIVE_MODE_1S))
        return false;

    c.present  = true;
    c.ready    = false;
    c.warming  = true;
    c.startMs  = millis();
    c.lastReadMs = 0;
    c.errorCount = 0;

    return true;
}

// -----------------------------
static bool ccs811ReadAll(uint8_t addr, CCS811Cache& c)
{
    uint8_t buf[8];

    if (!i2cRead(addr, CCS811_REG_ALG_RESULT, buf, 8))
    {
        c.errorCount++;
        return false;
    }

    c.eco2 = (buf[0] << 8) | buf[1];
    c.tvoc = (buf[2] << 8) | buf[3];

    uint32_t now = millis();

    if (now - c.startMs >= CCS811_WARMUP_MS)
    {
        c.warming = false;
        c.ready   = true;
    }

    c.lastReadMs = now;
    c.errorCount = 0;
    return true;
}

// -----------------------------
bool leerSignalCCS811(const Signal& s, float& out)
{
    uint8_t idx = s.address & 0x03;
    CCS811Cache& c = cache[idx];

    if (!c.present)
    {
        if (!ccs811Init(s.address, c))
            return false;
    }

    // Todavía calentando → no dar valor
    if (c.warming)
        return false;

    if (!c.ready)
        return false;

    if (millis() - c.lastReadMs > CCS811_CACHE_MS)
    {
        if (!ccs811ReadAll(s.address, c))
            return false;
    }

    switch ((CCS811Channel)s.channel)
    {
        case CCS811Channel::ECO2:
            out = (float)c.eco2;
            return true;

        case CCS811Channel::TVOC:
            out = (float)c.tvoc;
            return true;

        default:
            return false;
    }
}
