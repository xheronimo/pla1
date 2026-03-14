#include <WebServer.h>
#include <ArduinoJson.h>
#include "i2c_autodetect.h"

extern WebServer server;

static void handleI2CScan()
{
    auto list = i2cAutoDetect();

    DynamicJsonDocument doc(2048);
    JsonArray arr = doc.to<JsonArray>();

    for (auto& c : list)
    {
        JsonObject o = arr.createNestedObject();
        o["type"] = (int)c.type;
        o["address"] = c.address;
    }

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void registerI2CScanAPI()
{
    server.on("/api/i2c/scan", HTTP_GET, handleI2CScan);
}
