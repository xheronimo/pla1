#include "i2c_chip_guard.h"
#include "i2c_chip_registry.h"
#include "i2c_chip_context.h"
#include "system/LogSystem.h"
#include "i2c_recovery.h"

#include <Wire.h>

// ==================================================
// GUARD BEFORE READ
// ==================================================
bool i2cChipGuardBeforeRead(ChipContext *ctx, uint32_t now)
{
    if (!ctx)
        return false;

    if (!ctx->inUse)
        return false;

    // 🚫 Deshabilitado por configuración
    if (ctx->disabled)
        return false;

    // 🔒 Bloqueo definitivo (protección industrial)
    if (ctx->state == ChipState::STATE_LOCKED)
        return false;

    // ⛔ ERROR / TIMEOUT → backoff
    if (ctx->state == ChipState::STATE_ERROR ||
        ctx->state == ChipState::STATE_TIMEOUT)
    {
        if (now - ctx->initTs < ctx->retryMs)
            return false;

        // 🔄 Permitimos reintento
        ctx->state = ChipState::STATE_UNINITIALIZED;
        ctx->initTs = now;
        ctx->consecutiveErrors = 0;
        ctx->i2cFailCount = 0;
    }

    // 🕒 Warmup activo
    if (ctx->state == ChipState::STATE_WARMUP)
    {
        if (now - ctx->initTs < ctx->warmupMs)
            return false;

        ctx->state = ChipState::STATE_READY;
    }

    return true;
}

// ==================================================
// GUARD ON ERROR
// ==================================================
void i2cChipGuardOnError(ChipContext* ctx)
{
    if (!ctx)
        return;

    ctx->consecutiveErrors++;
    ctx->totalErrors++;

    // 🔐 Watchdog I2C por chip
    ctx->i2cFailCount++;
    ctx->lastI2cFailTs = millis();

    // ⚠️ Recovery físico del bus tras 3 fallos seguidos
    if (ctx->i2cFailCount >= 3)
    {
        escribirLog(
            "I2C: BUS FAIL chip %u addr 0x%02X (%u)",
            (uint8_t)ctx->type,
            ctx->address,
            ctx->i2cFailCount
        );

        // 🔧 Recuperación física
        Wire.end();
        i2cRecoverBus();
        Wire.begin();

        ctx->i2cFailCount = 0;
    }

    // ⚠️ Escalado de estado
    if (ctx->consecutiveErrors >= 2)
        ctx->state = ChipState::STATE_ERROR;
}

// ==================================================
// GUARD ON SUCCESS
// ==================================================
void i2cChipGuardOnSuccess(ChipContext *ctx, uint32_t now)
{
    if (!ctx)
        return;

    ctx->totalReads++;
    ctx->consecutiveErrors = 0;
    ctx->i2cFailCount = 0;

    ctx->state = ChipState::STATE_READY;
    ctx->lastReadTs = now;
}

// ==================================================
// RECOVERY DEL BUS (PULSOS SCL)
// ==================================================
static void i2cPulseClock()
{
    pinMode(SCL, OUTPUT);

    for (int i = 0; i < 9; i++)
    {
        digitalWrite(SCL, HIGH);
        delayMicroseconds(5);
        digitalWrite(SCL, LOW);
        delayMicroseconds(5);
    }

    pinMode(SCL, INPUT_PULLUP);
}

// ==================================================
// RECOVERY EXPLÍCITO (INVOCABLE)
// ==================================================
void i2cTryRecoverBus(ChipContext *ctx)
{
    if (!ctx)
        return;

    escribirLog(
        "I2C: Intentando recovery BUS (chip %u addr 0x%02X)",
        (uint8_t)ctx->type,
        ctx->address
    );

    Wire.end();
    i2cPulseClock();
    Wire.begin();

    // 🔄 Forzamos reinicialización limpia del chip
    ctx->state = ChipState::STATE_UNINITIALIZED;
    ctx->initTs = millis();
}
