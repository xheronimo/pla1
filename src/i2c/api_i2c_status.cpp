#include <WebServer.h>
#include <ArduinoJson.h>
#include "i2c/i2c_bus.h"
#include "i2c_metrics.h"

extern WebServer server;

void handleApiI2CStatus()
{
    JsonDocument doc;

    doc["ops"]     = i2cGetTotalOps();
    doc["errors"]  = i2cGetTotalErrors();
    doc["resets"]  = i2cGetTotalResets();
    doc["health"]  = i2cGetBusHealth();
    doc["speed"]   = i2cGetCurrentSpeed();

    String out;
    serializeJson(doc, out);

    server.send(200, "application/json", out);
}
