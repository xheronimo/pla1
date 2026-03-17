#include "web_api_watchdog_health.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "system/WatchdogManager.h"

static const char* wdtName(WatchdogId id) {
    switch (id) {
        case WDT_MAIN:    return "main";
        case WDT_SENSORS: return "sensors";
        case WDT_I2C:     return "i2c";
        case WDT_MODBUS:  return "modbus";
        case WDT_NET:     return "network";
        case WDT_MQTT:    return "mqtt";
        case WDT_DISPLAY: return "display";
        default:          return "other";
    }
}

void registerWatchdogHealthAPI(AsyncWebServer* server) {
    server->on("/api/watchdog/health", HTTP_GET, [](AsyncWebServerRequest *request) {
        watchdogKick(WDT_NET);

        JsonDocument doc;
        JsonArray arr = doc["watchdogs"].to<JsonArray>();

        for (int i = 0; i < WDT_MAX; i++) {
            WatchdogId id = (WatchdogId)i;
            if (!watchdogIsRegistered(id)) continue;

            JsonObject o = arr.add<JsonObject>();
            o["id"] = i;
            o["name"] = wdtName(id);
            o["lastKickMsAgo"] = watchdogLastKickAgo(id);
            
            // Añadimos estado visual para la web
            uint32_t ago = watchdogLastKickAgo(id);
            if (ago < 5000) o["status"] = "OK";
            else if (ago < 15000) o["status"] = "WARNING";
            else o["status"] = "STALE";
        }

        String out;
        serializeJson(doc, out);
        request->send(200, "application/json", out);
    });
}