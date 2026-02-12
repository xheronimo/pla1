#include "ccs811_driver.h"
#include <Arduino.h>
#include <Wire.h>

// ==============================
// REGISTROS
// ==============================
#define CCS811_REG_STATUS       0x00
#define CCS811_REG_MEAS_MODE    0x01
#define CCS811_REG_ALG_RESULT   0x02
#define CCS811_REG_ENV_DATA     0x05
#define CCS811_REG_APP_START    0xF4
#define CCS811_REG_SW_RESET     0xFF

// ==============================
// CACHE
// ==============================
struct Ccs811Cache {
    bool valid = false;
    uint16_t eco2 = 0;
    uint16_t tvoc = 0;
    uint32_t lastReadMs = 0;
};

static Ccs811Cache cache[8];

// ============================================================
// INIT
// ============================================================

bool ccs811Init(uint8_t addr, uint8_t options)
{
    ChipContext* ctx = i2cGetContext(I2CDevice::CCS811, addr);
    if (!ctx) return false;

    ctx->state = ChipState::INITIALIZING;
    ctx->initTs = millis();
    ctx->warmupMs = 20000;   // ⚠ CCS necesita ~20s reales
    ctx->retryMs = 5000;
    ctx->consecutiveErrors = 0;

    // Software reset
    uint8_t resetSeq[4] = {0x11, 0xE5, 0x72, 0x8A};
    i2cWriteBytes(addr, CCS811_REG_SW_RESET, resetSeq, 4);
    delay(5);

    // APP_START
    Wire.beginTransmission(addr);
    Wire.write(CCS811_REG_APP_START);
    if (Wire.endTransmission() != 0) {
        ctx->state = ChipState::ERROR;
        return false;
    }

    delay(5);

    // Measurement mode
    uint8_t mode;

    switch (options) {
        case 0: mode = 0x10; break; // 1 sec
        case 1: mode = 0x20; break; // 10 sec
        case 2: mode = 0x30; break; // 60 sec
        default: mode = 0x10; break;
    }

    if (!i2cWrite8(addr, CCS811_REG_MEAS_MODE, mode)) {
        ctx->state = ChipState::ERROR;
        return false;
    }

    ctx->state = ChipState::WARMUP;
    return true;
}

// ============================================================
// READ ALL
// ============================================================

static bool ccs811ReadAll(uint8_t addr, Ccs811Cache& c)
{
    uint8_t buf[4];

    if (!i2cReadBytes(addr, CCS811_REG_ALG_RESULT, buf, 4))
        return false;

    c.eco2 = (buf[0] << 8) | buf[1];
    c.tvoc = (buf[2] << 8) | buf[3];

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ============================================================
// API PRINCIPAL
// ============================================================

bool ccs811ReadSignal(const Signal& s, float& out)
{
    ChipContext* ctx = i2cGetContext(I2CDevice::CCS811, s.address);
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
        if (!ccs811Init(s.address, s.options)) {
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
    Ccs811Cache& c = cache[idx];

    if (!c.valid || now - c.lastReadMs > CCS811_CACHE_MS) {
        if (!ccs811ReadAll(s.address, c)) {
            ctx->consecutiveErrors++;
            if (ctx->consecutiveErrors >= 5)
                ctx->state = ChipState::ERROR;
            return false;
        }

        ctx->consecutiveErrors = 0;
        ctx->lastReadTs = now;
    }

    switch (s.channel) {
        case 0: out = c.eco2; return true;
        case 1: out = c.tvoc; return true;
        default: return false;
    }
}

// ============================================================
// METADATA
// ============================================================

void ccs811GetMetadata(ChipMetadata& meta)
{
    static const char* rates[] = {
        "1s",
        "10s",
        "60s"
    };

    meta.name = "CCS811";
    meta.channelCount = 2;

    meta.opt1.label = "Sample Rate";
    meta.opt1.values = rates;
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

void ccs811Reset()
{
    for (int i = 0; i < 8; i++)
        cache[i] = {};
}

bool ccs811Detect(uint8_t addr)
{
    uint8_t id;
    if (!i2cRead8(addr, 0x20, id))
        return false;

    return (id == 0x81);
}
