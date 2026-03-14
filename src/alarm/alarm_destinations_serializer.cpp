#include "alarm_destinations_serializer.h"
#include "alarm_destinations.h"
#include <ArduinoJson.h>

#include <string.h>

bool alarmDestinationsLoadFromJson(const JsonArray& arr)
{
    alarmDestinationCount = 0;

    for (JsonObject o : arr)
    {
        if (alarmDestinationCount >= MAX_ALARM_DESTINATIONS)
            break;

        AlarmDestination& d = alarmDestinations[alarmDestinationCount++];

        d.activo = o["EN"] | false;
        d.grupos = o["GR"] | 0;
        d.canales = o["CH"] | CH_NONE;
        d.enviarRecuperacion = o["REC"] | false;

        strlcpy(d.nombre, o["NAME"] | "", sizeof(d.nombre));
        strlcpy(d.telefono, o["TEL"] | "", sizeof(d.telefono));
        d.telegramChatId = o["TG"] | 0;
        strlcpy(d.mqttTopic, o["MQTT"] | "", sizeof(d.mqttTopic));
    }

    return true;
}

bool alarmDestinationsSaveToJson(JsonDocument& doc)
{
    JsonArray arr = doc["AD"].to<JsonArray>();

    for (uint8_t i = 0; i < alarmDestinationCount; i++)
    {
        const AlarmDestination& d = alarmDestinations[i];
        if (!d.activo) continue;

        JsonObject o = arr.add<JsonObject>();
        o["EN"] = d.activo;
        o["GR"] = d.grupos;
        o["CH"] = d.canales;
        o["REC"] = d.enviarRecuperacion;
        o["NAME"] = d.nombre;
        o["TEL"] = d.telefono;
        o["TG"] = d.telegramChatId;
        o["MQTT"] = d.mqttTopic;
    }

    return true;
}
