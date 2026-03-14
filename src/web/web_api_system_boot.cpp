#include "web_api_system_boot.h"
#include <ArduinoJson.h>
#include <WebServer.h>
#include "system/LogSystem.h"

extern WebServer server;
extern bool g_lastResetPower;
extern bool g_lastResetSoftware;
extern bool g_lastResetWdt;

void handleApiSystemBoot()
{
    JsonDocument doc;

    doc["power"] = g_lastResetPower;
    doc["software"] = g_lastResetSoftware;
    doc["watchdog"] = g_lastResetWdt;

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void registerSystemBootApi()
{
    server.on("/api/system/boot", HTTP_GET, handleApiSystemBoot);
}
