#include "i2c/ads1115_driver.h"
#include <Wire.h>

// ==============================
// REGISTROS ADS1115
// ==============================
#define ADS1115_REG_CONVERSION  0x00
#define ADS1115_REG_CONFIG      0x01

// Configuración:
// - PGA ±4.096V
// - Single-shot
// - 128 SPS
#define ADS1115_CFG_BASE  0x8583
#define ADS1115_CFG_CH0   0x4000
#define ADS1115_CFG_CH1   0x5000
#define ADS1115_CFG_CH2   0x6000
#define ADS1115_CFG_CH3   0x7000

#define ADS1115_LSB       0.000125f   // V/bit @ ±4.096V

// ==============================
// CACHE
// ==============================
struct Ads1115Cache {
    bool     valid = false;
    int16_t  raw[4];
    uint32_t lastReadMs = 0;
};

static Ads1115Cache cache[ADS1115_MAX_CHIPS];
static ChipContext  ctx[ADS1115_MAX_CHIPS];

// ==============================
// LOW LEVEL I2C
// ==============================
static bool i2cWrite16(uint8_t addr, uint8_t reg, uint16_t val)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val >> 8);
    Wire.write(val & 0xFF);
    return Wire.endTransmission() == 0;
}

static bool i2cRead16(uint8_t addr, uint8_t reg, uint16_t& out)
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

// ==============================
// READ ALL CHANNELS (NO BLOQUEANTE)
// ==============================
static bool ads1115ReadAll(uint8_t addr, Ads1115Cache& c)
{
    static const uint16_t muxCfg[4] = {
        ADS1115_CFG_CH0,
        ADS1115_CFG_CH1,
        ADS1115_CFG_CH2,
        ADS1115_CFG_CH3
    };

    for (uint8_t ch = 0; ch < 4; ch++)
    {
        uint16_t cfg = ADS1115_CFG_BASE | muxCfg[ch];

        if (!i2cWrite16(addr, ADS1115_REG_CONFIG, cfg))
            return false;

        // Espera mínima conversión (128 SPS ≈ 8ms → usamos polling simple)
        uint32_t t0 = millis();
        uint16_t raw;

        while (millis() - t0 < 10)
        {
            if (i2cRead16(addr, ADS1115_REG_CONVERSION, raw))
            {
                c.raw[ch] = (int16_t)raw;
                break;
            }
        }
    }

    c.valid      = true;
    c.lastReadMs = millis();
    return true;
}

// ==============================
// API PRINCIPAL
// ==============================
bool leerSignalADS1115(const Signal& s, float& out)
{
    if (s.channel > 3)
        return false;

    uint8_t idx = s.address & 0x07;
    if (idx >= ADS1115_MAX_CHIPS)
        return false;

    Ads1115Cache& c = cache[idx];
    ChipContext&  cc = ctx[idx];
    uint32_t now = millis();

    // ---------------- INIT ----------------
    if (cc.state == ChipState::UNINITIALIZED)
    {
        cc.state    = ChipState::INITIALIZING;
        cc.initTs   = now;
        cc.warmupMs = ADS1115_WARMUP_MS;
        return false;
    }

    if (cc.state == ChipState::INITIALIZING)
    {
        if (now - cc.initTs >= cc.warmupMs)
            cc.state = ChipState::READY;
        else
            return false;
    }

    if (cc.state == ChipState::ERROR)
        return false;

    // ---------------- CACHE ----------------
    if (!c.valid || now - c.lastReadMs > ADS1115_CACHE_MS)
    {
        if (!ads1115ReadAll(s.address, c))
        {
            cc.errorCount++;
            if (cc.errorCount > 5)
                cc.state = ChipState::ERROR;
            return false;
        }
        cc.errorCount = 0;
    }

    // RAW → VOLTS
    out = c.raw[s.channel] * ADS1115_LSB;
    cc.lastReadTs = now;
    return true;
}

// ==============================
// RESET
// ==============================
void ads1115Reset()
{
    for (uint8_t i = 0; i < ADS1115_MAX_CHIPS; i++)
    {
        cache[i] = {};
        ctx[i]   = {};
    }
}
