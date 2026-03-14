#include "web_api_health.h"
#include "system/system_health.h"
#include "system/boot_reason.h"
#include <Arduino.h>

#include <WebServer.h>
#include <ArduinoJson.h>

extern WebServer server;

extern bool g_lastResetPower;
extern bool g_lastResetSoftware;
extern bool g_lastResetWdt;

void handleApiHealth()
{
    SystemHealth h = getSystemHealth();
    JsonDocument doc;

    doc["modbus"]["ok"] = h.modbusOk;
    doc["modbus"]["errors"] = h.modbusErrors;

    doc["i2c"]["ok"] = h.i2cOk;
    doc["i2c"]["errors"] = h.i2cErrors;
    doc["boot"]["wdt"] = g_lastResetWdt;
doc["boot"]["power"] = g_lastResetPower;
doc["boot"]["soft"] = g_lastResetSoftware;


    doc["uptime"] = h.uptimeSec;

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void registerHealthApi()
{
    server.on("/api/health", HTTP_GET, handleApiHealth);
}
