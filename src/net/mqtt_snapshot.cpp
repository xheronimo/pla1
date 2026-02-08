#include "mqtt_snapshot.h"
#include "mqtt_manager.h"
#include "net/mqtt_topics.h"

#include "alarm/alarm_runtime.h"
#include "alarm/alarm_utils.h"
#include "alarm/alarm_registry.h"
#include <ArduinoJson.h>
#include "mqtt_topics.h"


void mqttPublishAlarmSnapshot()
{
    StaticJsonDocument<1024> doc;
    JsonArray arr = doc.createNestedArray("alarms");

    size_t count = 0;
    const AlarmRule* rules = alarmRegistryAll(count);

    for (size_t i = 0; i < count; i++)
    {
        const AlarmRule& r = rules[i];
        AlarmRuntime* rt = alarmRuntimeGet(r.alarmId);

        JsonObject o = arr.createNestedObject();
        o["id"] = r.alarmId;
        o["severity"] = alarmSeverityToStr(r.severity);
        o["group"] = alarmGroupToStr(r.group);

        o["active"] = rt ? rt->active : false;
        o["acked"]  = rt ? rt->acked  : false;
    }

    char buffer[1024];
    serializeJson(doc, buffer);

    MQTTManager::publish(MQTT_TOPIC_ALARM_SNAPSHOT, buffer, true);
}

/*
void mqttPublishSignalSnapshot()
{
    // ordenar por group + order
    sortSignals();

    for (auto& s : signals)
    {
        StaticJsonDocument<256> doc;
        doc["group"] = s.group;
        doc["order"] = s.order;
        doc["label"] = s.label;
        doc["value"] = s.value;
        doc["unit"]  = s.unit;
        doc["error"] = s.error;

        publish(MQTT_TOPIC_SIGNALS, doc);
    }
}
    */