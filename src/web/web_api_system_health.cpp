#include "web_api_system_health.h"
#include <Arduino.h>
#include <ArduinoJson.h>

#include "i2c/i2c_metrics.h"
#include <WebServer.h>
#include "esp_system.h"
#include "system/WatchdogManager.h"
#include "system/boot_reason.h"
#include "system/system_fault.h"
#include "system/system_safe_mode.h"


extern WebServer server;
extern bool g_modbusDegraded;

static const char* resetReasonToStr(esp_reset_reason_t r)
{
    switch (r)
    {
        case ESP_RST_POWERON:   return "power_on";
        case ESP_RST_SW:        return "software";
        case ESP_RST_PANIC:     return "panic";
        case ESP_RST_TASK_WDT:  return "task_wdt";
        case ESP_RST_WDT:       return "wdt";
        case ESP_RST_BROWNOUT:  return "brownout";
        default:                return "other";
    }
}

void handleApiSystemHealth()
{
    watchdogKick(WDT_NET);

    JsonDocument doc;

    // -------------------------------------------------
    // SISTEMA
    // -------------------------------------------------
    doc["uptimeMs"] = millis();
    doc["heapFree"] = ESP.getFreeHeap();
    doc["heapMin"]  = ESP.getMinFreeHeap();

    // -------------------------------------------------
    // BOOT / RESET
    // -------------------------------------------------
    JsonObject boot = doc["boot"].to<JsonObject>();
    boot["reason"]     = resetReasonToStr(esp_reset_reason());
    boot["wdt"]        = g_lastResetWdt;
    boot["power"]      = g_lastResetPower;
    boot["software"]   = g_lastResetSoftware;
    boot["wdtCount"]   = g_wdtResetCount;

    // -------------------------------------------------
    // MODBUS
    // -------------------------------------------------
    JsonObject modbus = doc["modbus"].to<JsonObject>();
    modbus["degraded"] = g_modbusDegraded;

    // -------------------------------------------------
    // ESTADO CRÍTICO GLOBAL
    // -------------------------------------------------
    doc["criticalFault"] = systemHasCriticalFault();

    doc["safeMode"] = systemInSafeMode();

    // -------------------------------------------------
    // WATCHDOGS
    // -------------------------------------------------
    JsonArray wdt = doc["watchdogs"].to<JsonArray>();
    for (int i = 0; i < WDT_MAX; i++)
    {
        WatchdogId id = (WatchdogId)i;

        if (!watchdogIsRegistered(id))
            continue;

        JsonObject o = wdt.add<JsonObject>();
        o["id"] = i;
        o["lastKickMsAgo"] = watchdogLastKickAgo(id);
    }

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}


void registerSystemHealthAPI()
{
    server.on("/api/system/health", HTTP_GET, handleApiSystemHealth);
}
