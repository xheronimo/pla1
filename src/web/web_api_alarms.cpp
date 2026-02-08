#include "web_api_alarms.h"
#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>


#include "alarm/alarm_config.h"
#include "alarm/alarm_runtime.h"
#include "alarm/alarm_registry.h"
#include "signal/signal_manager.h"
#include "alarm/alarm_utils.h"

extern WebServer server;
void handleApiGetAlarms()
{
    size_t count;
    const AlarmRule* rules = alarmRegistryAll(count);

    JsonDocument doc;
    JsonArray arr = doc["alarms"].to<JsonArray>();

    for (size_t i = 0; i < count; i++)
    {
        const AlarmRule& r = rules[i];

        JsonObject o = arr.add<JsonObject>();
        o["id"]     = r.alarmId;
        o["active"] = alarmRuntimeIsActive(r.alarmId);
        o["acked"]  = alarmRuntimeIsAcked(r.alarmId);
        o["group"]  = alarmGroupToStr(r.group);
        o["level"]  = alarmSeverityToStr(r.severity);
    }

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}


