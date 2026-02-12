#include "i2c/ina219_driver.h"
#include <Arduino.h>

// ==============================
// REGISTROS
// ==============================
#define INA219_REG_CONFIG      0x00
#define INA219_REG_SHUNT_VOLT  0x01
#define INA219_REG_BUS_VOLT    0x02

// ==============================
// CONFIG HW
// ==============================
constexpr float INA219_SHUNT_OHMS = 0.1f;
constexpr float INA219_BUS_LSB    = 0.004f;     // 4mV
constexpr float INA219_SHUNT_LSB  = 0.00001f;   // 10uV

// ==============================
// CACHE
// ==============================
struct Ina219Cache {
    bool valid = false;
    float voltage = 0.0f;
    float current = 0.0f;
    float power   = 0.0f;
    uint32_t lastReadMs = 0;
};

static Ina219Cache cache[8];


// ============================================================
// INIT
// ============================================================

bool ina219Init(uint8_t addr, uint8_t options)
{
    ChipContext* ctx = i2cGetContext(I2CDevice::INA219, addr);
    if (!ctx) return false;

    ctx->state = ChipState::INITIALIZING;
    ctx->initTs = millis();
    ctx->warmupMs = 2;
    ctx->retryMs = 2000;
    ctx->consecutiveErrors = 0;

    uint16_t config;

    if (options == 1) {
        // 32V range + 128 samples (máxima estabilidad)
        config = 0x399F;
    } else {
        // 16V range + 128 samples (default)
        config = 0x019F;
    }

    if (!i2cWrite16(addr, INA219_REG_CONFIG, config)) {
        ctx->state = ChipState::ERROR;
        return false;
    }

    ctx->state = ChipState::READY;
    return true;
}


// ============================================================
// READ ALL
// ============================================================

static bool ina219ReadAll(uint8_t addr, Ina219Cache& c)
{
    uint16_t rawBus;
    uint16_t rawShunt;

    if (!i2cRead16(addr, INA219_REG_BUS_VOLT, rawBus))
        return false;

    if (!i2cRead16(addr, INA219_REG_SHUNT_VOLT, rawShunt))
        return false;

    rawBus >>= 3;
    c.voltage = rawBus * INA219_BUS_LSB;

    int16_t shunt = (int16_t)rawShunt;
    float shuntV = shunt * INA219_SHUNT_LSB;

    c.current = shuntV / INA219_SHUNT_OHMS;
    c.power   = c.voltage * c.current;

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}


// ============================================================
// API PRINCIPAL
// ============================================================

bool ina219ReadSignal(const Signal& s, float& out)
{
    ChipContext* ctx = i2cGetContext(I2CDevice::INA219, s.address);
    if (!ctx) return false;

    uint32_t now = millis();

    // ---------------- SAFE ----------------
    if (ctx->state == ChipState::ERROR) {
        if (now - ctx->lastReadTs > ctx->retryMs) {
            ctx->state = ChipState::UNINITIALIZED;
            ctx->consecutiveErrors = 0;
        }
        return false;
    }

    // ---------------- INIT CASCADA ----------------
    if (ctx->state == ChipState::UNINITIALIZED) {
        if (!ina219Init(s.address, s.options)) {
            ctx->consecutiveErrors++;
            if (ctx->consecutiveErrors >= 5)
                ctx->state = ChipState::ERROR;
            return false;
        }
        return false;
    }

    if (ctx->state != ChipState::READY)
        return false;

    uint8_t idx = s.address & 0x07;
    Ina219Cache& c = cache[idx];

    if (!c.valid || now - c.lastReadMs > INA219_CACHE_MS)
    {
        if (!ina219ReadAll(s.address, c)) {
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
        case 0: out = c.voltage; return true;
        case 1: out = c.current; return true;
        case 2: out = c.power;   return true;
        default: return false;
    }
}


// ============================================================
// METADATA
// ============================================================

void ina219GetMetadata(ChipMetadata& meta)
{
    static const char* ranges[] = {
        "16V Range",
        "32V Range"
    };

    meta.name = "INA219 Power Monitor";
    meta.channelCount = 3;

    meta.opt1.label = "Voltage Range";
    meta.opt1.values = ranges;
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

void ina219Reset()
{
    for (int i = 0; i < 8; i++)
        cache[i] = {};
}
bool ina219Detect(uint8_t addr)
{
    uint16_t config;
    return i2cRead16(addr, 0x00, config);
}
