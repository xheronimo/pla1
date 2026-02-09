#include "i2c/ads1115_driver.h"
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// =================================================
// REGISTROS
// =================================================
#define ADS_REG_CONVERSION  0x00
#define ADS_REG_CONFIG      0x01

// =================================================
// CACHE
// =================================================
struct Ads1115Cache {
    bool     valid = false;
    int16_t  raw[4] = {0};
    uint32_t lastReadMs = 0;
};

static Ads1115Cache cache[ADS1115_MAX_CHIPS];
static ChipContext  ctx[ADS1115_MAX_CHIPS];

// =================================================
// MUTEX I2C
// =================================================
static SemaphoreHandle_t i2cMutex = nullptr;

static void ensureMutex()
{
    if (!i2cMutex)
        i2cMutex = xSemaphoreCreateMutex();
}

// =================================================
// UTILIDADES CONFIG
// =================================================
static uint16_t gainBits(Ads1115Gain g)
{
    switch (g) {
        case Ads1115Gain::GAIN_6_144V: return 0x0000;
        case Ads1115Gain::GAIN_4_096V: return 0x0200;
        case Ads1115Gain::GAIN_2_048V: return 0x0400;
        case Ads1115Gain::GAIN_1_024V: return 0x0600;
        case Ads1115Gain::GAIN_0_512V: return 0x0800;
        default:                      return 0x0A00;
    }
}

static uint16_t rateBits(Ads1115Rate r)
{
    return ((uint16_t)r & 0x07) << 5;
}

static float lsbFromGain(Ads1115Gain g)
{
    switch (g) {
        case Ads1115Gain::GAIN_6_144V: return 0.0001875f;
        case Ads1115Gain::GAIN_4_096V: return 0.000125f;
        case Ads1115Gain::GAIN_2_048V: return 0.0000625f;
        case Ads1115Gain::GAIN_1_024V: return 0.00003125f;
        case Ads1115Gain::GAIN_0_512V: return 0.000015625f;
        default:                      return 0.0000078125f;
    }
}

// =================================================
// LOW LEVEL
// =================================================
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

// =================================================
// INIT
// =================================================
bool ads1115Init(uint8_t addr, const Ads1115Options& opt)
{
    ensureMutex();

    uint8_t idx = addr & 0x07;
    ChipContext& c = ctx[idx];

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
        return false;

    c.state      = ChipState::INITIALIZING;
    c.initTs     = millis();
    c.warmupMs   = 2;
    c.retryMs    = 1000;
    c.errorCount = 0;

    // Modo continuo, AIN0 por defecto
    uint16_t cfg =
        0x0003 |              // CONTINUOUS
        0x0100 |              // AIN0
        gainBits(opt.gain) |
        rateBits(opt.rate);

    if (!i2cWrite16(addr, ADS_REG_CONFIG, cfg)) {
        c.state = ChipState::ERROR;
        xSemaphoreGive(i2cMutex);
        return false;
    }

    c.state = ChipState::READY;
    xSemaphoreGive(i2cMutex);
    return true;
}

// =================================================
// READ ALL CHANNELS
// =================================================
static bool ads1115ReadAll(uint8_t addr, Ads1115Cache& c)
{
    static const uint16_t mux[4] = {
        0x4000, 0x5000, 0x6000, 0x7000
    };

    for (uint8_t ch = 0; ch < 4; ch++)
    {
        uint16_t cfg = mux[ch] | 0x0003; // Continuous

        if (!i2cWrite16(addr, ADS_REG_CONFIG, cfg))
            return false;

        vTaskDelay(pdMS_TO_TICKS(2));

        uint16_t raw;
        if (!i2cRead16(addr, ADS_REG_CONVERSION, raw))
            return false;

        c.raw[ch] = (int16_t)raw;
    }

    c.valid      = true;
    c.lastReadMs = millis();
    return true;
}

// =================================================
// API PRINCIPAL
// =================================================
bool leerSignalADS1115(const Signal& s, float& out)
{
    ensureMutex();

    uint8_t idx = s.address & 0x07;
    Ads1115Cache& ac = cache[idx];
    ChipContext&  cc = ctx[idx];

    uint32_t now = millis();

    if (s.channel > 3)
        return false;

    if (cc.state == ChipState::UNINITIALIZED)
        return false;

    if (cc.state != ChipState::READY)
        return false;

    if (!ac.valid || now - ac.lastReadMs > ADS1115_CACHE_MS)
    {
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
            return false;

        bool ok = ads1115ReadAll(s.address, ac);

        xSemaphoreGive(i2cMutex);

        if (!ok) {
            cc.errorCount++;
            if (cc.errorCount > 5)
                cc.state = ChipState::ERROR;
            return false;
        }
    }

    Ads1115Gain gain = (Ads1115Gain)((s.options >> 4) & 0x07);
    float lsb = lsbFromGain(gain);

    out = ac.raw[s.channel] * lsb;
    return true;
}

// =================================================
// RESET
// =================================================
void ads1115Reset()
{
    for (uint8_t i = 0; i < ADS1115_MAX_CHIPS; i++) {
        cache[i] = {};
        ctx[i]   = {};
    }
}
