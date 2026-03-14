#include "web_api_watchdog_health.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "system/WatchdogManager.h"
#include <WebServer.h>
#include "esp_system.h"


extern WebServer server;
extern uint32_t g_lastKick[WDT_MAX];
extern bool     g_registered[WDT_MAX];

static const char* wdtName(WatchdogId id)
{
    switch (id)
    {
        case WDT_MAIN:      return "main";
        case WDT_SENSORS:   return "sensors";
        case WDT_I2C:       return "i2c";
        case WDT_MODBUS:    return "modbus";
        case WDT_NET:       return "network";
        case WDT_MQTT:      return "mqtt";
        case WDT_DISPLAY:  return "display";
        default:            return "other";
    }
}

static void handleApiWatchdogHealth()
{
    watchdogKick(WDT_NET);   // 🔐 importante

    JsonDocument doc;
    JsonArray arr = doc["watchdogs"].to<JsonArray>();

    for (int i = 0; i < WDT_MAX; i++)
    {
        if (!watchdogIsRegistered((WatchdogId)i))
            continue;

        JsonObject o = arr.add<JsonObject>();
        o["id"] = i;
        o["name"] = wdtName((WatchdogId)i);
        o["lastKickMsAgo"] = watchdogLastKickAgo((WatchdogId)i);
    }

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void registerWatchdogHealthAPI()
{
    server.on("/api/watchdog/health", HTTP_GET, handleApiWatchdogHealth);
}
