#include "web_api_modbus_health.h"
#include <WebServer.h>
#include <ArduinoJson.h>

#include "modbus/modbus_config.h"
#include "modbus/modbus_slave_context.h"
#include "modbus/modbus_guard.h"
#include "system/WatchdogManager.h"
#include "modbus/modbus_metrics.h"

extern WebServer server;

extern bool g_modbusDegraded;

static const char *stateToStr(ModbusSlaveState s)
{
    switch (s)
    {
    case ModbusSlaveState::STATE_UNINITIALIZED:
        return "UNINITIALIZED";
    case ModbusSlaveState::STATE_READY:
        return "READY";
    case ModbusSlaveState::STATE_ERROR:
        return "ERROR";
    case ModbusSlaveState::STATE_TIMEOUT:
        return "TIMEOUT";
    case ModbusSlaveState::STATE_DISABLED:
        return "DISABLED";
    default:
        return "UNKNOWN";
    }
}

void handleApiModbusHealth()
{
    watchdogKick(WDT_NET); // 🔐 feed watchdog de red

    JsonDocument doc;
    JsonArray slaves = doc["slaves"].to<JsonArray>();

    doc["degraded"] = g_modbusDegraded;
    doc["maxSlaves"] = MODBUS_MAX_SLAVES;

    for (uint8_t id = 1; id <= MODBUS_MAX_SLAVES; id++)
    {
        ModbusSlaveContext *ctx = modbusGetContext(id);
        if (!ctx || !ctx->inUse)
            continue;

        JsonObject o = slaves.add<JsonObject>();

        o["id"] = id;
        o["state"] = stateToStr(ctx->state);
        o["priority"] = (int)ctx->priority;
        o["critical"] = ctx->critical;

        o["pollMs"] = ctx->pollIntervalMs;
        o["lastPollAgo"] = (ctx->lastPollTs == 0) ? -1 : (int32_t)(millis() - ctx->lastPollTs);
        o["lastOkAgo"] = (ctx->lastOkTs == 0) ? -1 : (int32_t)(millis() - ctx->lastOkTs);

        o["errors"] = ctx->totalErrors;
        o["consecutive"] = ctx->consecutiveErrors;
        o["configErrors"] = ctx->configErrors;

        if (ctx->disabledUntilMs > millis())
            o["disabledForMs"] = ctx->disabledUntilMs - millis();
        else
            o["disabledForMs"] = 0;
        o["totalErrors"] = modbusGetTotalErrors();
        o["totalReads"] = modbusGetTotalReads();
    }

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void registerModbusHealthApi()
{
    server.on("/api/modbus/health", HTTP_GET, handleApiModbusHealth);
}
