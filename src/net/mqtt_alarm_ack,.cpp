#include "mqtt_alarm_ack.h"
#include "alarm/alarm_runtime.h"
#include "alarm/alarm_persistence.h"
#include <ArduinoJson.h>
#include "alarm/alarm_queue.h"

void handleAlarmAck(const char *payload)
{
    StaticJsonDocument<128> doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err)
        return;

    uint32_t alarmId = doc["alarmId"] | UINT32_MAX;
    if (alarmId == UINT32_MAX)
        return;

    // 1️⃣ Marcar ACK en runtime
    alarmRuntimeAck(alarmId);

    // 2️⃣ Persistir
    bool active = alarmRuntimeIsActive(alarmId);
    bool acked = alarmRuntimeIsAcked(alarmId);

    alarmPersistenceSave(alarmId, active, acked);
    
    AlarmEvent ev{};
    ev.alarmId = alarmId;
    ev.kind = AlarmEventKind::ACK;
    ev.active = alarmRuntimeIsActive(alarmId);

    alarm_queuePush(ev);
}

void handleAlarmAck(uint32_t alarmId)
{
    if (!alarmRuntimeIsActive(alarmId))
        return;

    alarmRuntimeAck(alarmId);

    AlarmEvent ev{};
    ev.alarmId = alarmId;
    ev.active = false;
    ev.kind = AlarmEventKind::ACK;

    alarm_queuePush(ev);
}