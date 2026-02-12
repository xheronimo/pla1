#include "bh1750_driver.h"
#include <Arduino.h>
#include "i2c_bus.h"

// ==============================
// COMANDOS BH1750
// ==============================
#define BH1750_POWER_ON          0x01
#define BH1750_RESET             0x07
#define BH1750_CONT_H_RES_MODE   0x10
#define BH1750_CONT_H_RES2_MODE  0x11
#define BH1750_CONT_L_RES_MODE   0x13

// ==============================
// CACHE
// ==============================
struct Bh1750Cache {
    bool valid = false;
    float lux = 0.0f;
    uint32_t lastReadMs = 0;
};

static Bh1750Cache cache[8];

// ============================================================
// INIT
// ============================================================

bool bh1750Init(uint8_t addr, uint8_t options)
{
    ChipContext* ctx = i2cGetContext(I2CDevice::BH1750, addr);
    if (!ctx) return false;

    ctx->state = ChipState::INITIALIZING;
    ctx->initTs = millis();
    ctx->warmupMs = 180;     // tiempo medición alta resolución
    ctx->retryMs = 2000;
    ctx->consecutiveErrors = 0;

    if (!i2cWriteRaw(addr, BH1750_POWER_ON)) {
        ctx->state = ChipState::ERROR;
        return false;
    }

    if (!i2cWriteRaw(addr, BH1750_RESET)) {
        ctx->state = ChipState::ERROR;
        return false;
    }

    uint8_t mode;

    switch (options) {
        case 0: mode = BH1750_CONT_H_RES_MODE; break;   // 1 lux
        case 1: mode = BH1750_CONT_H_RES2_MODE; break;  // 0.5 lux
        case 2: mode = BH1750_CONT_L_RES_MODE; break;   // 4 lux
        default: mode = BH1750_CONT_H_RES_MODE; break;
    }

    if (!i2cWriteRaw(addr, mode)) {
        ctx->state = ChipState::ERROR;
        return false;
    }

    ctx->state = ChipState::WARMUP;
    return true;
}

// ============================================================
// READ ALL
// ============================================================

static bool bh1750ReadAll(uint8_t addr, Bh1750Cache& c)
{
    uint8_t buf[2];

    if (!i2cReadRaw(addr, buf, 2))
        return false;

    uint16_t raw = (buf[0] << 8) | buf[1];
    c.lux = raw / 1.2f;  // Conversión estándar

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ============================================================
// API PRINCIPAL
// ============================================================

bool bh1750ReadSignal(const Signal& s, float& out)
{
    ChipContext* ctx = i2cGetContext(I2CDevice::BH1750, s.address);
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
        if (!bh1750Init(s.address, s.options)) {
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
    Bh1750Cache& c = cache[idx];

    if (!c.valid || now - c.lastReadMs > BH1750_CACHE_MS) {
        if (!bh1750ReadAll(s.address, c)) {
            ctx->consecutiveErrors++;
            if (ctx->consecutiveErrors >= 5)
                ctx->state = ChipState::ERROR;
            return false;
        }

        ctx->consecutiveErrors = 0;
        ctx->lastReadTs = now;
    }

    out = c.lux;
    return true;
}

// ============================================================
// METADATA
// ============================================================

void bh1750GetMetadata(ChipMetadata& meta)
{
    static const char* modes[] = {
        "High Resolution (1 lux)",
        "High Resolution 2 (0.5 lux)",
        "Low Resolution (4 lux)"
    };

    meta.name = "BH1750 Light Sensor";
    meta.channelCount = 1;

    meta.opt1.label = "Resolution Mode";
    meta.opt1.values = modes;
    meta.opt1.valueCount = 3;
    meta.opt1.defaultIndex = 0;

    meta.opt2.label = nullptr;
    meta.opt2.values = nullptr;
    meta.opt2.valueCount = 0;
    meta.opt2.defaultIndex = 0;
}

// ============================================================
// RESET
// ============================================================

void bh1750Reset()
{
    for (int i = 0; i < 8; i++)
        cache[i] = {};
}

bool bh1750Detect(uint8_t addr)
{
    uint8_t dummy;
    return i2cRead8(addr, 0x00, dummy);
}
