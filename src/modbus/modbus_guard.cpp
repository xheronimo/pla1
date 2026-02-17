#include "modbus_guard.h"
#include "system/system_mode.h"
#include "system/LogSystem.h"
#include <Arduino.h>

extern bool g_modbusDegraded;
bool modbusGuardBeforeRead(ModbusSlaveContext* ctx)
{
    if (!ctx)
        return false;

    if (ctx->state == ModbusSlaveState::STATE_DISABLED)
        return false;

    uint32_t now = millis();

    // -------- TIMEOUT / ERROR --------
    if (ctx->state == ModbusSlaveState::STATE_TIMEOUT ||
        ctx->state == ModbusSlaveState::STATE_ERROR)
    {
        if (now - ctx->lastOkTs >= ctx->retryMs)
        {
            ctx->state = ModbusSlaveState::STATE_UNINITIALIZED;
            ctx->consecutiveErrors = 0;
            ctx->disabledUntilMs = 0;

            escribirLog("MODBUS: Slave %u REINTENTO", ctx->id);
        }
        else
        {
            return false;
        }
    }

    return true;
}


void modbusGuardOnSuccess(ModbusSlaveContext *ctx)
{
    if (!ctx)
        return;
    g_modbusDegraded = false;
    ctx->state = ModbusSlaveState::STATE_READY;
    ctx->consecutiveErrors = 0;
    ctx->lastOkTs = millis();
    ctx->totalReads++;
}

void modbusGuardOnError(ModbusSlaveContext* ctx)
{
    if (!ctx)
        return;

    ctx->consecutiveErrors++;
    ctx->totalErrors++;

    if (ctx->consecutiveErrors >= MODBUS_ERROR_THRESHOLD)
    {
        if (ctx->state != ModbusSlaveState::STATE_TIMEOUT)
        {
            ctx->state = ModbusSlaveState::STATE_TIMEOUT;
            ctx->disabledUntilMs = millis() + ctx->retryMs;

            escribirLog("MODBUS: Slave %u TIMEOUT", ctx->id);
        }

        // 🔻 degradado global (una sola vez)
        if (!g_modbusDegraded)
        {
            g_modbusDegraded = true;
            escribirLog("MODBUS: Modo degradado ACTIVADO");
        }
    }
}


void modbusCheckConfigFault(ModbusSlaveContext *ctx)
{
    if (!ctx)
        return;

    if (ctx->configErrors >= 5)
        ctx->state = ModbusSlaveState::STATE_ERROR;
}
