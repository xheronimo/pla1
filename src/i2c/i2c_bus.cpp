#include "i2c_bus.h"
#include <Wire.h>
#include "board/board_config.h"
#include "system/WatchdogManager.h"
#include "system/RelojSistema.h"
#include "i2c_chip_context.h"
#include "i2c_chip_registry.h"
#include "system/LogSystem.h"
#include "i2c/i2c_chip_guard.h"
#include "i2c/pcf_driver.h"

// ========================================================
// CONFIG
// ========================================================

static SemaphoreHandle_t i2cMutex = nullptr;

// ========================================================
// INIT
// ========================================================

void i2cBusInit()
{
    if (!i2cMutex)
        i2cMutex = xSemaphoreCreateMutex();

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(400000); // 400kHz
    watchdogRegister(WDT_I2C);
}

SemaphoreHandle_t i2cGetMutex()
{
    return i2cMutex;
}

// ========================================================
// WRITE
// ========================================================

bool i2cWrite8(uint8_t addr, uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

bool i2cWrite16(uint8_t addr, uint8_t reg, uint16_t val)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val >> 8);
    Wire.write(val & 0xFF);
    return Wire.endTransmission() == 0;
}

bool i2cWriteBytes(uint8_t addr, uint8_t reg, const uint8_t *data, uint8_t len)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(data, len);
    return Wire.endTransmission() == 0;
}

bool i2cWriteRaw(uint8_t addr, uint8_t value)
{
    Wire.beginTransmission(addr);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

// ========================================================
// READ
// ========================================================

bool i2cRead8(uint8_t addr, uint8_t reg, uint8_t &out)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(addr, (uint8_t)1) != 1)
        return false;

    out = Wire.read();
    return true;
}

bool i2cRead16(uint8_t addr, uint8_t reg, uint16_t &out)
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

bool i2cReadBytes(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(addr, len) != len)
        return false;

    for (uint8_t i = 0; i < len; i++)
        buf[i] = Wire.read();

    return true;
}

bool i2cReadRaw(uint8_t addr, uint8_t *buf, uint8_t len)
{
    if (Wire.requestFrom(addr, len) != len)
        return false;

    for (uint8_t i = 0; i < len; i++)
        buf[i] = Wire.read();

    return true;
}

bool i2cPing(uint8_t addr)
{
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

ChipContext *i2cGetDriver(I2CDevice chip, uint8_t address)
{
    ChipContext *ctx = i2cGetChipContext(chip, address);
    if (!ctx)
        return nullptr;

    uint32_t now = millis();

    // 🔒 BLOQUEO DEFINITIVO (industrial)
    if (ctx->state == ChipState::STATE_LOCKED)
        return nullptr;

    // ⛔ ERROR / TIMEOUT → backoff antes de reintentar
    if (ctx->state == ChipState::STATE_ERROR ||
        ctx->state == ChipState::STATE_TIMEOUT)
    {
        if (ctx->totalErrors == 5)
        {
            i2cTryRecoverBus(ctx);
        }
        if (now - ctx->initTs < ctx->retryMs)
            return nullptr;

        // 🔄 Permitimos reintento
        ctx->state = ChipState::STATE_UNINITIALIZED;
        ctx->consecutiveErrors = 0;
        ctx->initTs = now;

        escribirLog(
            "I2C: Reintento chip %u addr 0x%02X",
            (uint8_t)chip,
            address);
    }

    // 🔄 INICIALIZACIÓN FÍSICA DEL CHIP
    if (ctx->state == ChipState::STATE_UNINITIALIZED)
    {
        if (!ctx->driver || !ctx->driver->init(
                                ctx->address,
                                ctx->options))
        {
            ctx->state = ChipState::STATE_ERROR;
            ctx->consecutiveErrors++;
            ctx->totalErrors++;
            ctx->initTs = now;

            escribirLog(
                "I2C: Init FALLÓ chip %u addr 0x%02X",
                (uint8_t)chip,
                address);

            return nullptr;
        }

        bool ok;

        if (ctx->type == I2CDevice::PCF8574 ||
            ctx->type == I2CDevice::PCF8575)
        {
            ok = pcfInit(ctx->address, ctx->options);
            if (ok)
                ok = pcfInitShadow(ctx);
        }
        else
        {
            ok = ctx->driver->init(ctx->address, ctx->options);
        }

        if (!ok)
        {
            ctx->state = ChipState::STATE_ERROR;
            ctx->totalErrors++;
            return nullptr;
        }

        ctx->state = ChipState::STATE_READY;
        ctx->shadowValid = true;

        // 🕒 Warmup si aplica
        if (ctx->warmupMs > 0)
        {
            ctx->state = ChipState::STATE_WARMUP;
            ctx->initTs = now;
        }
        else
        {
            ctx->state = ChipState::STATE_READY;
        }

        ctx->lastOkTs = now;
        ctx->consecutiveErrors = 0;
    }

    // 🕒 WARMUP ACTIVO
    if (ctx->state == ChipState::STATE_WARMUP)
    {
        if (now - ctx->initTs < ctx->warmupMs)
            return nullptr;

        ctx->state = ChipState::STATE_READY;
    }

    // 🚨 DEMASIADOS ERRORES → LOCK DEFINITIVO
    if (ctx->totalErrors >= 10)
    {
        ctx->state = ChipState::STATE_LOCKED;

        escribirLog(
            "I2C: Chip %u addr 0x%02X BLOQUEADO",
            (uint8_t)chip,
            address);

        return nullptr;
    }

    // ✅ SOLO AQUÍ SE PERMITE USO REAL DEL CHIP
    return ctx;
}

bool pcfInitShadow(ChipContext *ctx)
{
    if (!ctx)
        return false;

    uint16_t safe = 0xFFFF;

    // Limitar a 8 bits si es 8574
    if (ctx->type == I2CDevice::PCF8574)
        safe = 0x00FF;

    SemaphoreHandle_t m = i2cGetMutex();
    if (!m)
        return false;

    if (xSemaphoreTake(m, pdMS_TO_TICKS(20)) != pdTRUE)
        return false;

    Wire.beginTransmission(ctx->address);
    Wire.write(safe & 0xFF);

    if (ctx->type == I2CDevice::PCF8575)
        Wire.write((safe >> 8) & 0xFF);

    bool ok = (Wire.endTransmission() == 0);

    xSemaphoreGive(m);

    if (!ok)
        return false;

    ctx->shadow = safe;
    ctx->shadowValid = true;

    return true;
}
