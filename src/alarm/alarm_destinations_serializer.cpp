#include "alarm/alarm_destinations_serializer.h"
#include "alarm/alarm_destinations.h"
#include "system/LogSystem.h"
#include <string.h>

bool alarmDestinationsLoadFromJson(const JsonArray& arr) {
    alarmDestinationCount = 0;

    for (JsonObject o : arr) {
        if (alarmDestinationCount >= MAX_ALARM_DESTINATIONS) {
            escribirLog("ALARM: Limite de destinos superado en JSON");
            break;
        }

        AlarmDestination& d = alarmDestinations[alarmDestinationCount++];

        // Mapeo de claves cortas para optimizar espacio en SD
        d.activo             = o["EN"] | false;
        d.grupos             = o["GR"] | 0;
        d.canales            = o["CH"] | (uint8_t)CH_NONE;
        d.enviarRecuperacion = o["REC"] | false;

        strlcpy(d.nombre,    o["NAME"] | "", sizeof(d.nombre));
        strlcpy(d.telefono,  o["TEL"]  | "", sizeof(d.telefono));
        d.telegramChatId     = o["TG"]   | 0;
        strlcpy(d.mqttTopic, o["MQTT"] | "", sizeof(d.mqttTopic));
    }

    escribirLog("ALARM: %d destinos cargados desde JSON", alarmDestinationCount);
    return true;
}

bool alarmDestinationsSaveToJson(JsonDocument& doc) {
    // Creamos el array bajo la clave "AD" (Alarm Destinations)
    JsonArray arr = doc["AD"].to<JsonArray>();

    for (uint8_t i = 0; i < alarmDestinationCount; i++) {
        const AlarmDestination& d = alarmDestinations[i];
        
        // Guardamos todos los destinos registrados para no perder la config
        JsonObject o = arr.add<JsonObject>();
        o["EN"]   = d.activo;
        o["GR"]   = d.grupos;
        o["CH"]   = d.canales;
        o["REC"]  = d.enviarRecuperacion;
        o["NAME"] = d.nombre;
        o["TEL"]  = d.telefono;
        o["TG"]   = d.telegramChatId;
        o["MQTT"] = d.mqttTopic;
    }

    return true;
}