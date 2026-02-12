#include <WebServer.h>
#include <ArduinoJson.h>
#include "i2c_chip_registry.h"

extern WebServer server;

static void handleApiGetI2CDrivers()
{
    DynamicJsonDocument doc(4096);
    JsonArray arr = doc.to<JsonArray>();

    size_t count = i2cGetDriverCount();

    for (size_t i = 0; i < count; i++)
    {
        const I2CChipDriver* drv = i2cGetDriverByIndex(i);
        if (!drv)
            continue;

        ChipMetadata meta;
        drv->meta(meta);

        JsonObject o = arr.createNestedObject();
        o["type"] = (int)drv->type;
        o["name"] = meta.name;
        o["channels"] = meta.channelCount;
        o["autoDetect"] = drv->allowAutoDetect;
    }

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void registerI2CDriversAPI()
{
    server.on("/api/i2c/drivers", HTTP_GET, handleApiGetI2CDrivers);
}
