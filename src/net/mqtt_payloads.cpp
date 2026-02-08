#include "mqtt_payloads.h"
#include <ArduinoJson.h>
#include "mqtt_manager.h"
#include "mqtt_topics.h"
#include "alarm/alarm_utils.h"
#include "alarm/alarm_runtime.h"
#include "system/time_utils.h"

void mqttPublishAlarm(const AlarmEvent& ev)
{
    JsonDocument doc;

    doc["id"]     = ev.alarmId;
    doc["active"] = ev.active;
    doc["kind"]   = alarmEventKindToStr(ev.kind);

    if (ev.primarySignalId != 0)
        doc["signal"] = ev.primarySignalId;

    doc["value"] = ev.value;

    char buf[256];
    serializeJson(doc, buf);

    MQTTManager::publish(MQTT_TOPIC_ALARM_EVENT, buf, false);
}

void mqttPublishAlarmState(uint32_t alarmId, bool active, bool acked)
{
    JsonDocument doc;
    doc["alarmId"] = alarmId;
    doc["active"]  = active;
    doc["acked"]   = acked;

    char buffer[128];
    serializeJson(doc, buffer);

    MQTTManager::publish(MQTT_TOPIC_ALARM_STATE, buffer, true);
}

void mqttPublishAlarmAck(uint32_t alarmId)
{
    StaticJsonDocument<128> doc;

    doc["alarmId"] = alarmId;
    doc["acked"]   = true;
    doc["ts"]      = getTimestamp();

    char buffer[128];
    serializeJson(doc, buffer);

    MQTTManager::publish(MQTT_TOPIC_ALARM_ACK, buffer, true);
}

void mqttPublishAlarmEvent(const AlarmEvent& ev, const AlarmRule& rule)
{
    StaticJsonDocument<256> doc;

    doc["alarmId"]  = ev.alarmId;
    doc["severity"] = alarmSeverityToStr(rule.severity);
    doc["group"]    = alarmGroupToStr(rule.group);
    doc["active"]   = ev.active;
    doc["acked"]    = alarmRuntimeIsAcked(ev.alarmId);
    doc["value"]    = ev.value;
    doc["ts"]       = getTimestamp();

    char buffer[256];
    serializeJson(doc, buffer);

    MQTTManager::publish(MQTT_TOPIC_ALARM_EVENT, buffer, true);
}