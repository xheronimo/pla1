#include "i2c/sht31_driver.h"
#include <Arduino.h>

// ==============================
// COMANDOS
// ==============================
#define SHT31_CMD_SOFT_RESET   0x30A2
#define SHT31_CMD_HIGHREP      0x2400
#define SHT31_CMD_MEDREP       0x240B
#define SHT31_CMD_LOWREP       0x2416

// ==============================
// CACHE
// ==============================
struct Sht31Cache {
    bool valid = false;
    float temperature = 0.0f;
    float humidity    = 0.0f;
    uint32_t lastReadMs = 0;
};

static Sht31Cache cache[8];


// ============================================================
// INIT
// ============================================================

bool sht31Init(uint8_t addr, uint8_t options)
{
    ChipContext* ctx = i2cGetContext(I2CDevice::SHT31, addr);
    if (!ctx) return false;

    ctx->state = ChipState::INITIALIZING;
    ctx->initTs = millis();
    ctx->warmupMs = 15;     // datasheet ~15ms
    ctx->retryMs = 2000;
    ctx->consecutiveErrors = 0;

    // Soft reset
    if (!i2cWrite16(addr, 0x30, 0xA2)) {
        ctx->state = ChipState::ERROR;
        return false;
    }

    ctx->state = ChipState::WARMUP;
    return true;
}


// ============================================================
// READ ALL
// ============================================================

static uint16_t getMeasureCmd(uint8_t options)
{
    switch (options)
    {
        case 1: return SHT31_CMD_MEDREP;
        case 2: return SHT31_CMD_LOWREP;
        default: return SHT31_CMD_HIGHREP;
    }
}

static bool sht31ReadAll(uint8_t addr, Sht31Cache& c, uint8_t options)
{
    uint16_t cmd = getMeasureCmd(options);

    if (!i2cWrite16(addr, cmd >> 8, cmd & 0xFF))
        return false;

    delay(15); // conversión interna (corto, estable)

    uint8_t buf[6];
    if (!i2cReadBytes(addr, 0x00, buf, 6))
        return false;

    uint16_t rawT = (buf[0] << 8) | buf[1];
    uint16_t rawH = (buf[3] << 8) | buf[4];

    c.temperature = -45.0f + 175.0f * (rawT / 65535.0f);
    c.humidity    = 100.0f * (rawH / 65535.0f);

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}


// ============================================================
// API PRINCIPAL
// ============================================================

bool sht31ReadSignal(const Signal& s, float& out)
{
    ChipContext* ctx = i2cGetContext(I2CDevice::SHT31, s.address);
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
        if (!sht31Init(s.address, s.options)) {
            ctx->consecutiveErrors++;
            if (ctx->consecutiveErrors >= 5)
                ctx->state = ChipState::ERROR;
            return false;
        }
        return false;
    }

    // ---------------- WARMUP ----------------
    if (ctx->state == ChipState::WARMUP) {
        if (now - ctx->initTs < ctx->warmupMs)
            return false;
        ctx->state = ChipState::READY;
    }

    if (ctx->state != ChipState::READY)
        return false;

    uint8_t idx = s.address & 0x07;
    Sht31Cache& c = cache[idx];

    if (!c.valid || now - c.lastReadMs > SHT31_CACHE_MS)
    {
        if (!sht31ReadAll(s.address, c, s.options)) {
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
        case 0: out = c.temperature; return true;
        case 1: out = c.humidity;    return true;
        default: return false;
    }
}


// ============================================================
// METADATA
// ============================================================

void sht31GetMetadata(ChipMetadata& meta)
{
    static const char* reps[] = {
        "High Precision",
        "Medium Precision",
        "Low Precision"
    };

    meta.name = "SHT31 Temp/Humidity";
    meta.channelCount = 2;

    meta.opt1.label = "Repeatability";
    meta.opt1.values = reps;
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

void sht31Reset()
{
    for (int i = 0; i < 8; i++)
        cache[i] = {};
}
bool sht31Detect(uint8_t addr)
{
    uint16_t id;
    if (!i2cRead16(addr, 0xF3, id))
        return false;

    return (id == 0x0031);  // SHT31 ID
}
