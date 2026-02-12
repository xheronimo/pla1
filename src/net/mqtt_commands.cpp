#include "mqtt_commands.h"

#include <string.h>
#include <stdlib.h>

#include "net/mqtt_topics.h"
#include "alarm/alarm_runtime.h"

// --------------------------------------------------
// ACK de alarma
// --------------------------------------------------
static void handleAlarmAck(const char* payload)
{
    // payload = alarmId en texto ("12")
    uint32_t alarmId = atoi(payload);
    alarmRuntimeAck(alarmId);
}

// --------------------------------------------------
// Router de comandos MQTT
// --------------------------------------------------
void mqttOnMessage(const char* topic, const char* payload)
{
    if (strcmp(topic, MQTT_TOPIC_ALARM_ACK) == 0)
    {
        handleAlarmAck(payload);
        return;
    }

    // aquí añadirás más comandos en el futuro
}
