#include "mqtt_snapshot.h"
#include "mqtt_manager.h"
#include "net/mqtt_topics.h"

#include "alarm/alarm_runtime.h"
#include "alarm/alarm_utils.h"
#include "alarm/alarm_registry.h"
#include"system/system_fault.h"
#include "system/boot_reason.h"
#include <ArduinoJson.h>
#include "mqtt_topics.h"

extern bool g_lastResetWdt;
extern bool g_safeMode;
extern bool systemHasCriticalFault();

void mqttPublishAlarmSnapshot()
{
    StaticJsonDocument<1024> doc;

    // -----------------------------
    // METADATA SISTEMA
    // -----------------------------
    doc["uptimeMs"] = millis();
    doc["systemFault"] = systemHasCriticalFault();
    doc["lastResetWdt"] = g_lastResetWdt;

    // -----------------------------
    // ALARMAS
    // -----------------------------
    JsonArray arr = doc.createNestedArray("alarms");

    size_t count = 0;
    const AlarmRule* rules = alarmRegistryAll(count);

    for (size_t i = 0; i < count; i++)
    {
        const AlarmRule& r = rules[i];
        AlarmRuntime* rt = alarmRuntimeGet(r.alarmId);

        JsonObject o = arr.createNestedObject();
        o["id"]       = r.alarmId;
        o["severity"] = alarmSeverityToStr(r.severity);
        o["group"]    = alarmGroupToStr(r.group);

        o["active"]   = rt ? rt->active  : false;
        o["acked"]    = rt ? rt->acked   : false;
        o["blocked"]  = rt ? rt->blocked : true;
    }

    char buffer[1024];
    serializeJson(doc, buffer);

    MQTTManager::publish(
        MQTT_TOPIC_ALARM_SNAPSHOT,
        buffer,
        true   // retain
    );
}
