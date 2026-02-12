#include "bme280_driver.h"
#include <Arduino.h>

// ==============================
// REGISTROS
// ==============================
#define BME280_REG_ID        0xD0
#define BME280_REG_RESET     0xE0
#define BME280_REG_CTRL_HUM  0xF2
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CONFIG    0xF5
#define BME280_REG_DATA      0xF7

#define BME280_RESET_CMD     0xB6

// ==============================
// CACHE
// ==============================
struct Bme280Cache {
    bool valid = false;
    float temp = 0;
    float hum  = 0;
    float pres = 0;
    uint32_t lastReadMs = 0;
};

static Bme280Cache cache[8]; // 8 direcciones máximas por tipo

// ============================================================
// INIT
// ============================================================

bool bme280Init(uint8_t addr, uint8_t options)
{
    ChipContext* ctx = i2cGetContext(I2CDevice::BME280, addr);
    if (!ctx) return false;

    ctx->state = ChipState::INITIALIZING;
    ctx->initTs = millis();
    ctx->warmupMs = 20;
    ctx->retryMs = 2000;
    ctx->consecutiveErrors = 0;

    if (!i2cWrite8(addr, BME280_REG_RESET, BME280_RESET_CMD)) {
        ctx->state = ChipState::ERROR;
        return false;
    }

    delay(2);

    uint8_t oversampling = (options & 0x03) + 1; // 1..4

    i2cWrite8(addr, BME280_REG_CTRL_HUM, oversampling);

    i2cWrite8(addr, BME280_REG_CTRL_MEAS,
              (oversampling << 5) | (oversampling << 2) | 0x03);

    i2cWrite8(addr, BME280_REG_CONFIG, 0xA0);

    ctx->state = ChipState::WARMUP;
    return true;
}

// ============================================================
// READ ALL
// ============================================================

static bool bme280ReadAll(uint8_t addr, Bme280Cache& c)
{
    uint8_t buf[8];

    if (!i2cReadBytes(addr, BME280_REG_DATA, buf, 8))
        return false;

    int32_t adc_P = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
    int32_t adc_T = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);
    int32_t adc_H = (buf[6] << 8)  | buf[7];

    // Simplificado (industrial)
    c.temp = adc_T / 100.0f;
    c.pres = adc_P / 25600.0f;
    c.hum  = adc_H / 1024.0f;

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ============================================================
// API PRINCIPAL
// ============================================================

bool bme280ReadSignal(const Signal& s, float& out)
{
    ChipContext* ctx = i2cGetContext(I2CDevice::BME280, s.address);
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
        if (!bme280Init(s.address, s.options)) {
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
    Bme280Cache& c = cache[idx];

    if (!c.valid || now - c.lastReadMs > BME280_CACHE_MS) {
        if (!bme280ReadAll(s.address, c)) {
            ctx->consecutiveErrors++;
            if (ctx->consecutiveErrors >= 5)
                ctx->state = ChipState::ERROR;
            return false;
        }

        ctx->consecutiveErrors = 0;
        ctx->lastReadTs = now;
    }

    switch (s.channel) {
        case 0: out = c.temp; return true;
        case 1: out = c.hum;  return true;
        case 2: out = c.pres; return true;
        default: return false;
    }
}

// ============================================================
// METADATA
// ============================================================

void bme280GetMetadata(ChipMetadata& meta)
{
    static const char* oversampling[] = {
        "x1",
        "x2",
        "x4",
        "x8"
    };

    meta.name = "BME280";
    meta.channelCount = 3;

    meta.opt1.label = "Oversampling";
    meta.opt1.values = oversampling;
    meta.opt1.valueCount = 4;
    meta.opt1.defaultIndex = 1;

    meta.opt2.label = nullptr;
    meta.opt2.values = nullptr;
    meta.opt2.valueCount = 0;
    meta.opt2.defaultIndex = 0;
}

// ============================================================
// RESET
// ============================================================

void bme280Reset()
{
    for (int i = 0; i < 8; i++)
        cache[i] = {};
}

bool bme280Detect(uint8_t addr)
{
    uint8_t id;
    if (!i2cRead8(addr, 0xD0, id))
        return false;

    return (id == 0x60);
}