#include "WebUI.h"

#include <WebServer.h>
#include <ArduinoJson.h>

#include "alarm/alarm_runtime.h"
#include "alarm/alarm_ack.h"
#include "system/LogSystem.h"
#include "net/mqtt_payloads.h"
#include "web_alarm_api.h"
#include "web_api_alarms.h"
#include "web_api_alarm_ack.h"

static WebServer server(80);

// =====================================================

void handleStatus()
{
    server.send(200, "text/plain", "OK");
}


void handleAlarmAck()
{
    if (!server.hasArg("id"))
    {
        server.send(400, "text/plain", "Missing id");
        return;
    }

    uint32_t alarmId = server.arg("id").toInt();

    alarmRuntimeAck(alarmId);

    bool active = alarmRuntimeIsActive(alarmId);
    bool acked  = alarmRuntimeIsAcked(alarmId);

    mqttPublishAlarmState(alarmId, active, acked);

    server.send(200, "text/plain", "ACK OK");
}

// --------------------------------------------------
// INIT
// --------------------------------------------------
void webInit()
{
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/alarm/ack", HTTP_POST, handleAlarmAck);
    server.on("/api/alarm/ack", HTTP_POST, handleAlarmAck);
    server.on("/api/alarms", HTTP_GET, handleApiGetAlarms);
    server.on("/api/alarms/ack", HTTP_POST, handleApiAckAlarm);


    server.begin();
}

// --------------------------------------------------
// LOOP
// --------------------------------------------------
void webLoop()
{
    server.handleClient();
}
