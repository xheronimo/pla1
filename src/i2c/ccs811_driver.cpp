#include "i2c/ccs811_driver.h"
#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_chip_guard.h"
#include "i2c/i2c_bus.h"

#include <Arduino.h>
#include <Wire.h>

// ==============================
// REGISTROS
// ==============================
#define CCS811_REG_STATUS       0x00
#define CCS811_REG_MEAS_MODE    0x01
#define CCS811_REG_ALG_RESULT   0x02
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

// ==============================
// INIT
// ==============================

bool ccs811Init(uint8_t addr, uint8_t options)
{
    ChipContext* ctx = i2cGetChipContext(I2CDevice::CCS811, addr);
    if (!ctx) return false;

    ctx->state = ChipState::STATE_INITIALIZING;
    ctx->initTs = millis();
    ctx->warmupMs = 20000;   // 20 segundos reales
    ctx->retryMs = 5000;
    ctx->consecutiveErrors = 0;

    // Software reset
    uint8_t resetSeq[4] = {0x11, 0xE5, 0x72, 0x8A};
    if (!i2cWriteBytes(addr, CCS811_REG_SW_RESET, resetSeq, 4))
    {
        ctx->state = ChipState::STATE_ERROR;
        return false;
    }

    delay(5);

    // APP_START
    Wire.beginTransmission(addr);
    Wire.write(CCS811_REG_APP_START);
    if (Wire.endTransmission() != 0)
    {
        ctx->state = ChipState::STATE_ERROR;
        return false;
    }

    delay(5);

    // Measurement mode
    uint8_t mode;
    switch (options)
    {
        case 0: mode = 0x10; break; // 1s
        case 1: mode = 0x20; break; // 10s
        case 2: mode = 0x30; break; // 60s
        default: mode = 0x10; break;
    }

    if (!i2cWrite8(addr, CCS811_REG_MEAS_MODE, mode))
    {
        ctx->state = ChipState::STATE_ERROR;
        return false;
    }

    ctx->state = ChipState::STATE_WARMUP;
    return true;
}

// ==============================
// READ ALL
// ==============================

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

// ==============================
// READ SIGNAL
// ==============================

bool ccs811ReadSignal(const Signal& s, float& out)
{
    ChipContext* ctx = i2cGetChipContext(I2CDevice::CCS811, s.address);
    if (!ctx) return false;

    uint32_t now = millis();

    // 🔐 Guard central
    if (!i2cChipGuardBeforeRead(ctx, now))
        return false;

    // INIT cascada
    if (ctx->state == ChipState::STATE_UNINITIALIZED)
    {
        if (!ccs811Init(s.address, s.options))
        {
            i2cChipGuardOnError(ctx);
            return false;
        }
        return false; // esperar siguiente ciclo
    }

    // WARMUP gestionado por guard central

    Ccs811Cache& c = cache[s.address & 0x07];

    if (!c.valid || now - c.lastReadMs > CCS811_CACHE_MS)
    {
        if (!ccs811ReadAll(s.address, c))
        {
            i2cChipGuardOnError(ctx);
            return false;
        }
    }

    i2cChipGuardOnSuccess(ctx, now);

    switch (s.channel)
    {
        case 0: out = c.eco2; return true;
        case 1: out = c.tvoc; return true;
        default: return false;
    }
}

// ==============================
// METADATA
// ==============================

void ccs811GetMetadata(ChipMetadata& meta)
{
    static const char* rates[] = {
        "1s",
        "10s",
        "60s"
    };

    meta.name = "CCS811 Air Quality";
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

// ==============================
// RESET
// ==============================

void ccs811Reset()
{
    for (int i = 0; i < 8; i++)
        cache[i] = {};
}

// ==============================
// DETECT
// ==============================

bool ccs811Detect(uint8_t addr)
{
    uint8_t status;
    if (!i2cRead8(addr, CCS811_REG_STATUS, status))
        return false;

    return true;
}
