#include "net/mqtt_fault.h"
#include "net/mqtt_manager.h"
#include "system/system_fault.h"

static bool lastFaultState = false;

void mqttPublishFault(bool active)
{
    if (!MQTTManager::isConnected())
        return;

    MQTTManager::publish(
        "system/fault",
        active ? "1" : "0",
        true   // retain
    );
}

// Llamar periódicamente
void mqttFaultTick()
{
    bool now = systemHasCriticalFault();
    if (now != lastFaultState)
    {
        mqttPublishFault(now);
        lastFaultState = now;
    }
}
