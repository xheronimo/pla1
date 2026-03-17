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
// GUARDIA DE ERROR MEJORADA
// ==================================================
void i2cChipGuardOnError(ChipContext* ctx, ChipMode mode)
{
    if (!ctx) return;

    ctx->consecutiveErrors++;
    ctx->totalErrors++;
    ctx->i2cFailCount++;

    // 1. Rate Limiting de Logs (Evitar saturar SD)
    static uint32_t lastLogTs = 0;
    if (millis() - lastLogTs > 60000) {
        SDMgr::logEvent("ERROR", "I2C: Fallo de comunicación persistente.");
        lastLogTs = millis();
    }

    // 2. Recuperación Física (Tu lógica de 9 pulsos corregida)
    if (ctx->i2cFailCount >= 3) {
        i2cRecoverBus(); 
        ctx->i2cFailCount = 0;
    }

    // 3. Lógica de Seguridad por Modo
    if (ctx->consecutiveErrors >= 5) {
        ctx->state = ChipState::STATE_ERROR;

        // Si es un chip que controla salidas (Relés)
        if (mode == ChipMode::OUTPUT_ONLY || mode == ChipMode::MIXED) {
            // Marcamos que el control ya no es confiable
            ctx->shadowValid = false; 
            
            char msg[64];
            snprintf(msg, sizeof(msg), "CRITICAL: Pérdida de control en Chip 0x%02X", ctx->address);
            SDMgr::logEvent("CRITICAL", msg);
            
            // Aquí puedes disparar una bandera global de sistema:
            // systemSetSafeState(true); 
        }
    }
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
