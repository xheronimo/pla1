#include "web_api_system_health.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "esp_system.h"
#include "system/WatchdogManager.h"
#include "system/boot_reason.h"
#include "system/system_fault.h"
#include "system/system_safe_mode.h"

extern bool g_modbusDegraded;

static const char* resetReasonToStr(esp_reset_reason_t r) {
    switch (r) {
        case ESP_RST_POWERON:   return "power_on";
        case ESP_RST_SW:        return "software";
        case ESP_RST_PANIC:     return "panic";
        case ESP_RST_TASK_WDT:  return "task_wdt";
        case ESP_RST_WDT:       return "wdt";
        case ESP_RST_BROWNOUT:  return "brownout";
        default:                return "other";
    }
}

void registerSystemHealthAPI(AsyncWebServer* server) {
    server->on("/api/system/health", HTTP_GET, [](AsyncWebServerRequest *request) {
        watchdogKick(WDT_NET); // Mantenemos vivo el WDT de red durante la respuesta

        JsonDocument doc;

        // --- INFO SISTEMA ---
        doc["uptimeMs"] = millis();
        doc["heapFree"] = ESP.getFreeHeap();
        doc["heapMin"]  = ESP.getMinFreeHeap();

        // --- BOOT / RESET INFO ---
        JsonObject boot = doc["boot"].to<JsonObject>();
        boot["reason"]     = resetReasonToStr(esp_reset_reason());
        boot["wdt"]        = g_lastResetWdt;
        boot["power"]      = g_lastResetPower;
        boot["software"]   = g_lastResetSoftware;
        boot["wdtCount"]   = g_wdtResetCount;

        // --- MODBUS & FAULTS ---
        doc["modbusDegraded"] = g_modbusDegraded;
        doc["criticalFault"]  = systemHasCriticalFault();
        doc["safeMode"]       = systemInSafeMode();

        // --- WATCHDOGS (Resumen rápido) ---
        JsonArray wdt = doc["watchdogs"].to<JsonArray>();
        for (int i = 0; i < WDT_MAX; i++) {
            WatchdogId id = (WatchdogId)i;
            if (watchdogIsRegistered(id)) {
                JsonObject o = wdt.add<JsonObject>();
                o["id"] = i;
                o["lastKickMsAgo"] = watchdogLastKickAgo(id);
            }
        }

        String out;
        serializeJson(doc, out);
        request->send(200, "application/json", out);
    });
}