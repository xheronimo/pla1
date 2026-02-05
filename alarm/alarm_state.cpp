#include "alarm_state.h"
#include "alarm/alarm_rules.h"

void alarmStateToJson(JsonDocument& doc)
{
    JsonArray arr = doc.createNestedArray("ALARMS");

    alarmRulesForEach([&](const AlarmRule& r)
    {
        JsonObject o = arr.createNestedObject();
        o["ID"]     = r.id;
        o["MSG"]    = r.mensaje;
        o["ON"]     = r.activaAhora;
        o["LATCH"]  = r.latched;
        o["GROUP"]  = r.grupo;
    });
}
