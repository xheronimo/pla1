#include "web_api_i2c_health.h"

#include <WebServer.h>
#include <ArduinoJson.h>

#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_bus.h"
#include "i2c/i2c_watchdog.h"

extern WebServer server;

static void handleApiI2CHealth()
{
    JsonDocument doc;
    JsonArray arr = doc["chips"].to<JsonArray>();

    for (int t = 0; t < (int)I2CDevice::MAX; t++)
    {
        for (int i = 0; i < I2C_MAX_CHIPS_PER_TYPE; i++)
        {
            ChipContext *ctx = i2cGetChipContext((I2CDevice)t, i);
            if (!ctx)
                continue;

            if (ctx->totalReads == 0 && ctx->totalErrors == 0)
                continue; // no usado

            JsonObject o = arr.add<JsonObject>();

            o["type"] = t;
            o["addrIndex"] = i;
            o["state"] = (int)ctx->state;
            o["disabled"] = ctx->disabled;
            o["reads"] = ctx->totalReads;
            o["errors"] = ctx->totalErrors;
            o["consecutive"] = ctx->consecutiveErrors;
        }
    }

    doc["busStuck"] = i2cBusIsStuck();

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void registerI2CHealthAPI()
{
    server.on("/api/i2c/health", HTTP_GET, handleApiI2CHealth);
}
