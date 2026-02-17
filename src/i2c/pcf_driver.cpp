#include "pcf_driver.h"

#include <Wire.h>
#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_bus.h"
#include "i2c/i2c_chip_guard.h"
#include "system/LogSystem.h"

// =======================================================
// DETECT
// =======================================================
bool pcfDetect8574(uint8_t addr)
{
    Wire.beginTransmission(addr);
    return (Wire.endTransmission() == 0);
}

bool pcfDetect8575(uint8_t addr)
{
    Wire.beginTransmission(addr);
    return (Wire.endTransmission() == 0);
}

// =======================================================
// INIT (PCF8574 / PCF8575)
// =======================================================
bool pcfInit(uint8_t addr, uint8_t /*options*/)
{
    Wire.beginTransmission(addr);

    // Estado seguro: todo HIGH
    Wire.write(0xFF);
    Wire.write(0xFF); // no daña en 8574, necesario en 8575

    return (Wire.endTransmission() == 0);
}

// =======================================================
// READ SIGNAL
// =======================================================
bool pcfReadSignal(const Signal& s, float& out)
{
    ChipContext* ctx = i2cGetChipContext(s.chip, s.address);
    if (!ctx)
        return false;

    uint32_t now = millis();

    if (!i2cChipGuardBeforeRead(ctx, now))
        return false;

    SemaphoreHandle_t m = i2cGetMutex();
    if (!m)
        return false;

    if (xSemaphoreTake(m, pdMS_TO_TICKS(20)) != pdTRUE)
        return false;

    uint16_t value = 0;

    Wire.requestFrom((int)s.address,
                     (s.chip == I2CDevice::PCF8575) ? 2 : 1);

    if (Wire.available())
    {
        value = Wire.read();
        if (s.chip == I2CDevice::PCF8575 && Wire.available())
            value |= (Wire.read() << 8);
    }

    xSemaphoreGive(m);

    ctx->shadow = value;
    ctx->shadowValid = true;

    bool bit = (value >> s.channel) & 0x01;
    out = bit ? 1.0f : 0.0f;

    i2cChipGuardOnSuccess(ctx, now);
    return true;
}

// =======================================================
// WRITE SIGNAL
// =======================================================
bool pcfWriteSignal(const Signal& s, float value)
{
    if (!s.writable)
        return false;

    ChipContext* ctx = i2cGetChipContext(s.chip, s.address);
    if (!ctx)
        return false;

    uint32_t now = millis();

    if (!i2cChipGuardBeforeRead(ctx, now))
        return false;

    if (!ctx->shadowValid)
    {
        // No tocamos nada si no sabemos el estado
        i2cChipGuardOnError(ctx);
        return false;
    }

    SemaphoreHandle_t m = i2cGetMutex();
    if (!m)
        return false;

    if (xSemaphoreTake(m, pdMS_TO_TICKS(20)) != pdTRUE)
        return false;

    uint16_t outv = ctx->shadow;
    bool high = (value > 0.5f);

    if (high)
        outv |= (1 << s.channel);
    else
        outv &= ~(1 << s.channel);

    Wire.beginTransmission(s.address);
    Wire.write(outv & 0xFF);

    if (s.chip == I2CDevice::PCF8575)
        Wire.write((outv >> 8) & 0xFF);

    bool ok = (Wire.endTransmission() == 0);

    xSemaphoreGive(m);

    if (!ok)
    {
        i2cChipGuardOnError(ctx);
        return false;
    }

    ctx->shadow = outv;
    ctx->shadowValid = true;

    i2cChipGuardOnSuccess(ctx, now);
    return true;
}

// =======================================================
// METADATA
// =======================================================
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
