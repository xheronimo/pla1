#include "api_i2c_create.h"
#include <WebServer.h>
#include <ArduinoJson.h>
#include "signal/signal_factory.h"



extern WebServer server;


static void handleI2CCreate()
{
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Missing body");
        return;
    }

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, server.arg("plain"));

    I2CDevice type = (I2CDevice)doc["type"].as<int>();
    uint8_t addr   = doc["address"];
    bool reserved  = doc["reserved"] | false;

    const char* chipName = i2cGetDriverName(type);
    if (!chipName) {
        server.send(400, "text/plain", "Unknown I2C device");
        return;
    }

    for (JsonVariant v : doc["channels"].as<JsonArray>()) {
        uint8_t ch = v.as<uint8_t>();

        char id[32];
        char name[32];

        // ID técnico (estable)
        snprintf(id, sizeof(id),
                 "%s_%02X_CH%u",
                 chipName,
                 addr,
                 ch);

        // Nombre humano
        snprintf(name, sizeof(name),
                 "I2C %s %u",
                 chipName,
                 ch);

        addSystemSignal(
            id,
            name,
            type,
            addr,
            ch,
            BusType::BUS_I2C,
            SignalKind::SENSOR_ANALOG,
            900 + addr,   // deviceId lógico
            reserved,     // systemReserved
            reserved      // lockedConfig
        );
    }

    server.send(200, "text/plain", "OK");
}

void registerI2CCreateAPI()
{
    server.on("/api/i2c/create", HTTP_POST, handleI2CCreate);
}

