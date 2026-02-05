#include "alarm_snapshot.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

void alarmSnapshotSave()
{
    StaticJsonDocument<512> doc;

    JsonArray arr = doc.createNestedArray("rules");
    for (auto& r : alarmRuntimeAll())
    {
        JsonObject o = arr.createNestedObject();
        o["id"] = r.id;
        o["latched"] = r.latched;
        o["active"]  = r.activaAhora;
    }

    File f = LittleFS.open("/alarm_state.json", "w");
    serializeJson(doc, f);
    f.close();
}

void alarmSnapshotLoad()
{
    File f = LittleFS.open("/alarm_state.json", "r");
    if (!f) return;

    StaticJsonDocument<512> doc;
    deserializeJson(doc, f);

    for (auto o : doc["rules"].as<JsonArray>())
    {
        alarmRestore(
            o["id"],
            o["latched"],
            o["active"]
        );
    }
}
