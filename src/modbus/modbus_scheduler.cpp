#include "modbus_scheduler.h"
#include "modbus_manager.h"
#include "modbus_guard.h"
#include "modbus_slave_context.h"
#include "signal/signal_registry.h"
#include "modbus/modbus_config.h"
#include "modbus_decode.h"
#include "modbus_utils.h"
#include "system/system_mode.h"
#include "modbus_poll.h"
#include <Arduino.h>
#include <vector>

extern bool g_modbusDegraded;


void modbusPollScheduler()
{
    uint32_t now = millis();

    for (uint8_t slave = 1; slave <= MODBUS_MAX_SLAVES; slave++)
    {
        ModbusSlaveContext* ctx = modbusGetContext(slave);
        if (!ctx || !ctx->inUse)
            continue;

        // 🛑 Guard global del slave
        if (!modbusGuardBeforeRead(ctx))
            continue;

        // ⏱️ Intervalo base del slave
        uint32_t pollInterval = ctx->pollIntervalMs;

        // 🔻 MODO DEGRADADO → menos tráfico
        if (g_modbusDegraded)
            pollInterval *= 3;

        // ⏳ Aún no toca
        if (now - ctx->lastPollTs < pollInterval)
            continue;

        // 📍 Marcamos polling
        ctx->lastPollTs = now;

        // ---- POLLING REAL ----
        pollSlaveByType(slave, ModbusRegType::MODBUS_HOLDING);
        pollSlaveByType(slave, ModbusRegType::MODBUS_INPUT);
    }
}