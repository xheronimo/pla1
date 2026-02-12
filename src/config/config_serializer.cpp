#include "config/config_serializer.h"

bool saveConfigToJson(const Configuracion& cfg, JsonDocument& doc)
{
    // ---------- META ----------
    doc["ID"]    = cfg.meta.id;
    doc["MODEL"] = cfg.meta.model;
    doc["FW"]    = cfg.meta.fw;
    doc["BUILD"] = cfg.meta.build;

    // ---------- STATE ----------
    doc["STATE"]["SYSTEM"] = cfg.state.system;
    doc["STATE"]["ALARMS"] = cfg.state.alarms;
    doc["STATE"]["SMS"]    = cfg.state.sms;

    // ---------- NETWORK ----------
    doc["NET"]["APN"]  = cfg.network.apn;
    doc["NET"]["USER"] = cfg.network.user;
    doc["NET"]["PASS"] = cfg.network.pass;
    doc["NET"]["PIN"]  = cfg.network.pin;

    // ---------- MODEM ----------
    doc["MD"]["EN"] = cfg.modem.enabled;

    // ---------- MQTT ----------
    doc["MQ"]["EN"]   = cfg.mqtt.enabled;
    doc["MQ"]["HOST"] = cfg.mqtt.host;
    doc["MQ"]["PORT"] = cfg.mqtt.port;
    doc["MQ"]["USER"] = cfg.mqtt.user;
    doc["MQ"]["PASS"] = cfg.mqtt.pass;
    doc["MQ"]["BASE"] = cfg.mqtt.baseTopic;

    // ---------- TELEGRAM ----------
    doc["TG"]["EN"]    = cfg.telegram.enabled;
    doc["TG"]["TOKEN"] = cfg.telegram.token;

    // ---------- RTC ----------
    doc["RTC"]["EN"] = cfg.rtc.enabled;
    doc["RTC"]["TZ"] = cfg.rtc.timezone;

    // ---------- DESTINATIONS ----------
    JsonArray arr = doc.createNestedArray("AD");

    for (int i = 0; i < MAX_ALARM_DESTINATIONS; i++)
    {
        const AlarmDestination& d = cfg.alarmDestinations[i];
        JsonObject o = arr.createNestedObject();

        o["EN"]    = d.activo;
        o["CH"]    = d.canales;
        o["G"]     = d.grupos;
        o["REC"]   = d.enviarRecuperacion;
        o["NAME"]  = d.nombre;
        o["TEL"]   = d.telefono;
        o["TG"]    = d.telegramChatId;
        o["TOPIC"] = d.mqttTopic;
    }

    return true;
}
