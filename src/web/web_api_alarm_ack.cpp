#include "web_api_alarm_ack.h"
#include <Arduino.h>
#include <WebServer.h>

#include "alarm/alarm_runtime.h"

extern WebServer server;

void handleApiAckAlarm()
{
    if (!server.hasArg("alarmId"))
    {
        server.send(400, "text/plain", "alarmId missing");
        return;
    }

    uint32_t alarmId = server.arg("alarmId").toInt();
    alarmRuntimeAck(alarmId);

    server.send(200, "text/plain", "OK");
}
