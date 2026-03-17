#include "web_api_system_boot.h"
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "system/LogSystem.h"

extern uint32_t g_wdtResetCount;
extern bool g_lastResetPower;
extern bool g_lastResetSoftware;
extern bool g_lastResetWdt;

void registerSystemBootApi(AsyncWebServer* server) {
    // API de Boot Info
    server->on("/api/system/boot", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["power"] = g_lastResetPower;
        doc["software"] = g_lastResetSoftware;
        doc["watchdog"] = g_lastResetWdt;
        
        String out;
        serializeJson(doc, out);
        request->send(200, "application/json", out);
    });

    // API para resetear contador de Watchdog en NVS
    server->on("/api/system/reset_wdt", HTTP_POST, [](AsyncWebServerRequest *request) {
        Preferences prefs;
        prefs.begin("boot", false);
        g_wdtResetCount = 0;
        prefs.putUInt("wdtCount", 0);
        prefs.end();

        escribirLog("API: Contador WDT reseteado por usuario");
        request->send(200, "application/json", "{\"ok\":true}");
    });
}