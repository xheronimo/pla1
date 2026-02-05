#include "MQTTManager.h"
#include <ArduinoJson.h>

void mqttPublishDiscovery(const AlarmRule& r)
{
    StaticJsonDocument<256> doc;

    char uid[32];
    snprintf(uid, sizeof(uid), "alarm_%s", r.id);

    doc["name"] = r.mensaje;
    doc["uniq_id"] = uid;
    doc["stat_t"] = String("alarms/event/") + r.id;
    doc["val_tpl"] = "{{ value_json.estado }}";
    doc["pl_on"]  = "ON";
    doc["pl_off"] = "OFF";
    doc["json_attr_t"] = String("alarms/event/") + r.id;

    char topic[128];
    snprintf(topic, sizeof(topic),
        "homeassistant/binary_sensor/%s/config", uid);

    char payload[256];
    serializeJson(doc, payload);

    MQTTManager::publish(topic, payload, true);
}
