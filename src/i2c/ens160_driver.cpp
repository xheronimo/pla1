#include "ens160_driver.h"
#include <Arduino.h>

// ==============================
// REGISTROS
// ==============================
#define ENS160_REG_OPMODE     0x10
#define ENS160_REG_DATA_AQI   0x21
#define ENS160_REG_DATA_TVOC  0x22
#define ENS160_REG_DATA_ECO2  0x24
#define ENS160_REG_TEMP_IN    0x13
#define ENS160_REG_RH_IN      0x15

#define ENS160_OPMODE_RESET   0xF0
#define ENS160_OPMODE_IDLE    0x01
#define ENS160_OPMODE_STD     0x02

// ==============================
// CACHE
// ==============================
struct Ens160Cache {
    bool valid = false;
    uint16_t aqi = 0;
    uint16_t tvoc = 0;
    uint16_t eco2 = 0;
    uint32_t lastReadMs = 0;
};

static Ens160Cache cache[8];

// ============================================================
// INIT
// ============================================================

bool ens160Init(uint8_t addr, uint8_t options)
{
    ChipContext* ctx = i2cGetContext(I2CDevice::ENS160, addr);
    if (!ctx) return false;

    ctx->state = ChipState::INITIALIZING;
    ctx->initTs = millis();
    ctx->warmupMs = 60000;     // ⚠ ENS160 necesita ~60s reales
    ctx->retryMs = 5000;
    ctx->consecutiveErrors = 0;

    // RESET
    if (!i2cWrite8(addr, ENS160_REG_OPMODE, ENS160_OPMODE_RESET)) {
        ctx->state = ChipState::ERROR;
        return false;
    }

    delay(5);

    uint8_t mode = (options == 1)
        ? ENS160_OPMODE_IDLE
        : ENS160_OPMODE_STD;

    if (!i2cWrite8(addr, ENS160_REG_OPMODE, mode)) {
        ctx->state = ChipState::ERROR;
        return false;
    }

    ctx->state = ChipState::WARMUP;
    return true;
}

// ============================================================
// READ ALL
// ============================================================

static bool ens160ReadAll(uint8_t addr, Ens160Cache& c)
{
    uint16_t v;

    if (!i2cRead16(addr, ENS160_REG_DATA_AQI, v)) return false;
    c.aqi = v;

    if (!i2cRead16(addr, ENS160_REG_DATA_TVOC, v)) return false;
    c.tvoc = v;

    if (!i2cRead16(addr, ENS160_REG_DATA_ECO2, v)) return false;
    c.eco2 = v;

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ============================================================
// API PRINCIPAL
// ============================================================

bool ens160ReadSignal(const Signal& s, float& out)
{
    ChipContext* ctx = i2cGetContext(I2CDevice::ENS160, s.address);
    if (!ctx) return false;

    uint32_t now = millis();

    // SAFE
    if (ctx->state == ChipState::ERROR) {
        if (now - ctx->lastReadTs > ctx->retryMs) {
            ctx->state = ChipState::UNINITIALIZED;
            ctx->consecutiveErrors = 0;
        }
        return false;
    }

    // INIT CASCADA
    if (ctx->state == ChipState::UNINITIALIZED) {
        if (!ens160Init(s.address, s.options)) {
            ctx->consecutiveErrors++;
            if (ctx->consecutiveErrors >= 5)
                ctx->state = ChipState::ERROR;
            return false;
        }
        return false;
    }

    // WARMUP
    if (ctx->state == ChipState::WARMUP) {
        if (now - ctx->initTs < ctx->warmupMs)
            return false;

        ctx->state = ChipState::READY;
    }

    if (ctx->state != ChipState::READY)
        return false;

    uint8_t idx = s.address & 0x07;
    Ens160Cache& c = cache[idx];

    if (!c.valid || now - c.lastReadMs > ENS160_CACHE_MS) {
        if (!ens160ReadAll(s.address, c)) {
            ctx->consecutiveErrors++;
            if (ctx->consecutiveErrors >= 5)
                ctx->state = ChipState::ERROR;
            return false;
        }

        ctx->consecutiveErrors = 0;
        ctx->lastReadTs = now;
    }

    switch (s.channel)
    {
        case 0: out = c.aqi;  return true;
        case 1: out = c.tvoc; return true;
        case 2: out = c.eco2; return true;
        default: return false;
    }
}

// ============================================================
// COMPENSACIÓN AMBIENTAL
// ============================================================

bool ens160SetEnvironmentalData(uint8_t addr, float temp, float hum)
{
    uint16_t rh = (uint16_t)(hum * 64.0f);
    uint16_t t  = (uint16_t)((temp + 273.15f) * 64.0f);

    if (!i2cWrite16(addr, ENS160_REG_RH_IN, rh))
        return false;

    if (!i2cWrite16(addr, ENS160_REG_TEMP_IN, t))
        return false;

    return true;
}

// ============================================================
// METADATA
// ============================================================

void ens160GetMetadata(ChipMetadata& meta)
{
    static const char* modes[] = {
        "Standard Mode",
        "Idle Mode"
    };

    meta.name = "ENS160 Air Quality";
    meta.channelCount = 3;

    meta.opt1.label = "Operating Mode";
    meta.opt1.values = modes;
    meta.opt1.valueCount = 2;
    meta.opt1.defaultIndex = 0;

    meta.opt2.label = nullptr;
    meta.opt2.values = nullptr;
    meta.opt2.valueCount = 0;
    meta.opt2.defaultIndex = 0;
}

// ============================================================
// RESET
// ============================================================

void ens160Reset()
{
    for (int i = 0; i < 8; i++)
        cache[i] = {};
}

bool ens160Detect(uint8_t addr)
{
    uint16_t id;
    if (!i2cRead16(addr, 0x00, id))
        return false;

    return (id == 0x0160);
}
