#include "web_manager.h"

#include <WebServer.h>
#include <ArduinoJson.h>

#include "alarm/alarm_runtime.h"
#include "alarm/alarm_ack.h"
#include "net/mqtt_payloads.h"

#include "web_alarm_api.h"
#include "web_api_alarms.h"
#include "web_api_alarm_ack.h"
#include "signal/api_signal_guard.h"
#include "i2c/api_i2c_create.h"
#include "i2c/api_i2c_scan.h"
#include "i2c/api_i2c_drivers.h"






WebServer server(80);

// =====================================================
// HANDLERS BÁSICOS
// =====================================================

static void handleStatus()
{
    server.send(200, "text/plain", "OK");
}

void handleWebAlarmAck()
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

// =====================================================
// INIT
// =====================================================

void webInit()
{
    server.on("/status", HTTP_GET, handleStatus);

        server.on("/api/alarm/ack", HTTP_POST, handleWebAlarmAck);

    server.on("/api/alarms", HTTP_GET, handleApiGetAlarms);
    server.on("/api/alarms/ack", HTTP_POST, handleApiAckAlarm);

    

    registerI2CScanAPI();
registerI2CCreateAPI();
registerI2CDriversAPI();

    server.begin();
}

// =====================================================
// LOOP
// =====================================================

void webLoop()
{
    server.handleClient();
}
