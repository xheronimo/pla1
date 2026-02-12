#include "i2c/pcf_driver.h"
#include <string.h>
#include <Wire.h>

// ==============================
// CACHE POR CHIP
// ==============================
struct PcfCache
{
    bool     valid = false;
    uint16_t value = 0;
    uint32_t lastReadMs = 0;
};

static PcfCache cache[PCF_MAX_CHIPS];

// ==============================
// INIT
// ==============================

bool pcfInit(uint8_t addr, uint8_t options)
{
    ChipContext* ctx = i2cGetContext(I2CDevice::PCF8574, addr);
    if (!ctx) return false;

    if (!i2cDetect(addr))
    {
        ctx->state = ChipState::ERROR;
        return false;
    }

    ctx->state = ChipState::READY;
    ctx->consecutiveErrors = 0;
    return true;
}

// ==============================
// LOW LEVEL READ (8 o 16 bits)
// ==============================

static bool readRaw(uint8_t addr, I2CDevice chip, uint16_t& out)
{
    SemaphoreHandle_t m = i2cGetMutex();
    if (!m) return false;

    if (xSemaphoreTake(m, pdMS_TO_TICKS(20)) != pdTRUE)
        return false;

    bool ok = false;

    if (chip == I2CDevice::PCF8574)
    {
        uint8_t v;
        if (Wire.requestFrom(addr, (uint8_t)1) == 1)
        {
            v = Wire.read();
            out = v;
            ok = true;
        }
    }
    else if (chip == I2CDevice::PCF8575)
    {
        if (Wire.requestFrom(addr, (uint8_t)2) == 2)
        {
            uint8_t lo = Wire.read();
            uint8_t hi = Wire.read();
            out = (hi << 8) | lo;
            ok = true;
        }
    }

    xSemaphoreGive(m);
    return ok;
}

// ==============================
// READ
// ==============================

bool pcfReadSignal(const Signal& s, float& out)
{
    ChipContext* ctx = i2cGetContext(s.chip, s.address);
    if (!ctx) return false;

    uint32_t now = millis();

    // SAFE MODE
    if (ctx->state == ChipState::ERROR)
    {
        if (!ctx->fatal && now - ctx->lastReadTs > ctx->retryMs)
        {
            ctx->state = ChipState::UNINITIALIZED;
            ctx->consecutiveErrors = 0;
        }
        return false;
    }

    if (ctx->state == ChipState::UNINITIALIZED)
    {
        if (!pcfInit(s.address, s.options))
            return false;
    }

    uint8_t idx = s.address & 0x07;
    if (idx >= PCF_MAX_CHIPS)
        return false;

    PcfCache& c = cache[idx];

    if (!c.valid || now - c.lastReadMs > PCF_CACHE_MS)
    {
        if (!readRaw(s.address, s.chip, c.value))
        {
            ctx->consecutiveErrors++;
            ctx->errorCount++;

            if (ctx->consecutiveErrors >= 5)
                ctx->state = ChipState::ERROR;

            return false;
        }

        c.valid = true;
        c.lastReadMs = now;
        ctx->consecutiveErrors = 0;
        ctx->lastReadTs = now;
    }

    uint8_t bit = s.channel;

    if (s.chip == I2CDevice::PCF8574 && bit > 7)
        return false;

    if (s.chip == I2CDevice::PCF8575 && bit > 15)
        return false;

    bool level = (c.value >> bit) & 0x01;

    out = level ? 1.0f : 0.0f;
    return true;
}

// ==============================
// WRITE (SALIDA DIGITAL)
// ==============================

bool pcfWriteSignal(const Signal& s, float value)
{
    if (!s.writable)
        return false;

    SemaphoreHandle_t m = i2cGetMutex();
    if (!m) return false;

    if (xSemaphoreTake(m, pdMS_TO_TICKS(20)) != pdTRUE)
        return false;

    uint8_t addr = s.address;
    uint8_t bit  = s.channel;

    bool high = (value > 0.5f);

    uint16_t current = 0;
    readRaw(addr, s.chip, current);

    if (high)
        current |= (1 << bit);
    else
        current &= ~(1 << bit);

    bool ok = false;

    if (s.chip == I2CDevice::PCF8574)
    {
        ok = (Wire.beginTransmission(addr),
              Wire.write((uint8_t)current),
              Wire.endTransmission() == 0);
    }
    else if (s.chip == I2CDevice::PCF8575)
    {
        ok = (Wire.beginTransmission(addr),
              Wire.write(current & 0xFF),
              Wire.write((current >> 8) & 0xFF),
              Wire.endTransmission() == 0);
    }

    xSemaphoreGive(m);
    return ok;
}

// ==============================
// METADATA
// ==============================
void pcf8574GetMetadata(ChipMetadata& meta)
{
    meta.name = "PCF8574";
    meta.channelCount = 8;
}

void pcf8575GetMetadata(ChipMetadata& meta)
{
    meta.name = "PCF8575";
    meta.channelCount = 16;
}


// ==============================
// RESET
// ==============================

void pcfReset()
{
    memset(cache, 0, sizeof(cache));
}

bool pcfDetect8574(uint8_t addr)
{
    if (addr < 0x20 || addr > 0x27)
        return false;

    uint8_t value;
    return i2cRead8(addr, 0x00, value);
}

bool pcfDetect8575(uint8_t addr)
{
    if (addr < 0x20 || addr > 0x27)
        return false;

    uint16_t value;
    return i2cRead16(addr, 0x00, value);
}
