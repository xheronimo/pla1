#include "alarm_destinations_serializer.h"
#include "config/config_global.h"

extern Configuracion cfg;

bool alarmDestinationsSaveToJson(JsonDocument& doc)
{
    JsonArray arr = doc.createNestedArray("AD");

    for (int i = 0; i < MAX_ALARM_DESTINATIONS; i++)
    {
        const AlarmDestination& d = cfg.alarmDestinations[i];

        JsonObject o = arr.createNestedObject();

        o["EN"]  = d.activo ? 1 : 0;
        o["CH"]  = d.canales;
        o["G"]   = d.grupos;
        o["N"]   = d.nombre;

        if (d.telefono[0])
            o["TEL"] = d.telefono;

        if (d.telegramChatId != 0)
            o["TG"] = d.telegramChatId;

        if (d.mqttTopic[0])
            o["MQTT"] = d.mqttTopic;
    }

    return true;
}
