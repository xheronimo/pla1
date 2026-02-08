#include "alarm_registry.h"
#include "alarm_utils.h"


#define MAX_ALARMS 32

static AlarmRule registry[MAX_ALARMS];
static size_t registryCount = 0;

void alarmRegistryInit()
{
    registryCount = 0;
}

bool alarmRegistryAdd(const AlarmRule& rule)
{
    if (registryCount >= MAX_ALARMS)
        return false;

    registry[registryCount++] = rule;
    return true;
}

const AlarmRule* alarmRegistryGet(uint32_t alarmId)
{
    for (size_t i = 0; i < registryCount; i++)
    {
        if (registry[i].alarmId == alarmId)
            return &registry[i];
    }
    return nullptr;
}

const AlarmRule* alarmRegistryAll(size_t& count)
{
    count = registryCount;
    return registry;
}

void alarmRegistryClear()
{
    for (size_t i = 0; i < registryCount; i++)
    {
        freeExpr(registry[i].expr);
    }
    registryCount = 0;
}

size_t alarmRegistryCount(){
    return registryCount;
}

