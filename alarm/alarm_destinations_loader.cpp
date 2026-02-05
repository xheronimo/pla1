#include "alarm_destinations_loader.h"
#include "alarm_destination.h"
#include "Configuracion.h"
#include "system/LogSystem.h"

extern Configuracion cfg;

static uint8_t parseChannels(const JsonArray& arr)
{
    uint8_t ch = 0;

    for (const char* s : arr)
    {
        if (!strcmp(s, "SMS"))      ch |= CH_SMS;
        if (!strcmp(s, "TELEGRAM")) ch |= CH_TELEGRAM;
        if (!strcmp(s, "MQTT"))     ch |= CH_MQTT;
    }
    return ch;
}

bool alarmDestinationsLoadFromJson(const JsonArray& arr)
{
    for (int i = 0; i < MAX_ALARM_DESTINATIONS; i++)
        cfg.alarmDestinations[i].activo = false;

    int idx = 0;

    for (JsonObject o : arr)
    {
        if (idx >= MAX_ALARM_DESTINATIONS)
            break;

        AlarmDestination& d = cfg.alarmDestinations[idx++];

        d.activo = o["activo"] | false;
        d.grupos = o["grupos"] | 0;
        d.canales = parseChannels(o["canales"].as<JsonArray>());
        d.enviarRecuperacion = o["recuperacion"] | false;

        strlcpy(d.telefono,     o["telefono"]    | "", sizeof(d.telefono));
        strlcpy(d.telegramChat,o["telegram"]    | "", sizeof(d.telegramChat));
        strlcpy(d.mqttTopic,   o["mqtt_topic"]  | "", sizeof(d.mqttTopic));
    }

    escribirLog("ALM:DST:%d", idx);
    return true;
}
