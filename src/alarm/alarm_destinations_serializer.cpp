#include "alarm_destinations_serializer.h"
#include "alarm/alarm_destination.h"
#include "config/config_global.h"


// ==================================================
// LOAD
// ==================================================

bool alarmDestinationsLoadFromJson(const JsonArray& arr)
{
    int idx = 0;

    for (JsonObject o : arr)
    {
        if (idx >= MAX_ALARM_DESTINATIONS)
            break;

        AlarmDestination& d = cfg.alarmDestinations[idx++];

        d.activo = o["EN"] | false;
        d.grupos = o["GR"] | 0;
        d.canales = o["CH"] | CH_NONE;
        d.enviarRecuperacion = o["REC"] | false;

        strlcpy(d.nombre,   o["NAME"] | "", sizeof(d.nombre));
        strlcpy(d.telefono, o["TEL"]  | "", sizeof(d.telefono));

        d.telegramChatId = o["CHAT"] | 0;

        strlcpy(d.mqttTopic, o["TOP"] | "", sizeof(d.mqttTopic));
    }

    return true;
}

// ==================================================
// SAVE
// ==================================================

bool alarmDestinationsSaveToJson(JsonDocument& doc)
{
    JsonArray arr = doc.createNestedArray("AD");

    for (int i = 0; i < MAX_ALARM_DESTINATIONS; i++)
    {
        const AlarmDestination& d = cfg.alarmDestinations[i];
        if (!d.activo) continue;

        JsonObject o = arr.createNestedObject();

        o["EN"]   = d.activo;
        o["GR"]   = d.grupos;
        o["CH"]   = d.canales;
        o["REC"]  = d.enviarRecuperacion;

        o["NAME"] = d.nombre;
        o["TEL"]  = d.telefono;
        o["CHAT"] = d.telegramChatId;
        o["TOP"]  = d.mqttTopic;
    }

    return true;
}
