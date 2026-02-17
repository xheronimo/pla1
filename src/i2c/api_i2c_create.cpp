#include "api_i2c_create.h"
#include <WebServer.h>
#include <ArduinoJson.h>

#include "i2c_signal_factory.h"


extern WebServer server;

static void handleI2CCreate()
{
    if (!server.hasArg("plain"))
    {
        server.send(400, "text/plain", "Missing body");
        return;
    }

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));

    int type = doc["type"];
    int addr = doc["address"];

    std::vector<uint8_t> channels;
    for (JsonVariant v : doc["channels"].as<JsonArray>())
        channels.push_back(v.as<uint8_t>());

    bool ok = createSignalsForChip((I2CDevice)type, addr, channels);

    server.send(ok ? 200 : 500, "text/plain", ok ? "OK" : "FAIL");
}

void registerI2CCreateAPI()
{
    server.on("/api/i2c/create", HTTP_POST, handleI2CCreate);
}

