#include "web_alarm_api.h"
#include <WebServer.h>
#include <Arduino.h>

#include "alarm/alarm_runtime.h"

extern WebServer server;

// POST /api/alarm/ack?id=123
void handleAlarmAck()
{
    if (!server.hasArg("id"))
    {
        server.send(400, "text/plain", "missing id");
        return;
    }

    uint32_t alarmId = server.arg("id").toInt();

    alarmRuntimeAck(alarmId);

    server.send(200, "text/plain", "ok");
}
