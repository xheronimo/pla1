#include "alarm_latch_storage.h"
#include "alarm/alarm_rules.h"
#include "system/LogSystem.h"

#include <LittleFS.h>
#include <ArduinoJson.h>

// ===============================
// CARGAR LATCHES
// ===============================
bool alarmLatchLoad()
{
    if (!LittleFS.exists("/alarms_latched.json"))
        return true;

    File f = LittleFS.open("/alarms_latched.json", "r");
    if (!f) return false;

    StaticJsonDocument<1024> doc;
    if (deserializeJson(doc, f))
    {
        f.close();
        return false;
    }
    f.close();

    JsonArray arr = doc.as<JsonArray>();
    for (const char* id : arr)
    {
        alarmRulesForceLatch(id);
    }

    escribirLog("ALM:LATCH:RESTORED");
    return true;
}

// ===============================
// GUARDAR LATCHES
// ===============================
void alarmLatchSave()
{
    StaticJsonDocument<1024> doc;
    JsonArray arr = doc.to<JsonArray>();

    alarmRulesForEach([&](const AlarmRule& r) {
        if (r.latched)
            arr.add(r.id);
    });

    File f = LittleFS.open("/alarms_latched.json", "w");
    if (!f) return;

    serializeJson(doc, f);
    f.close();
}
