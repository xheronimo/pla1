#include "alarm_persistence.h"

#include "system/nvs_store.h"
#include "alarm_runtime.h"
#include "alarm_struct.h"
#include "alarm_registry.h"



void alarmPersistenceSave(uint32_t alarmId, bool active, bool acked)
{
    auto& nvs = NVS::alarms();

    char key[32];
    snprintf(key, sizeof(key), "ack_%lu", alarmId);

    nvs.putBool(key, acked);
}

void alarmPersistenceLoadAll()
{
    auto& nvs = NVS::alarms();

    size_t count;
    const AlarmRule* rules = alarmRegistryAll(count);

    for (size_t i = 0; i < count; i++)
    {
        const AlarmRule& r = rules[i];

        char key[32];
        snprintf(key, sizeof(key), "ack_%lu", r.alarmId);

        bool acked = nvs.getBool(key, false);

        // 🔁 RESTORE aquí
        alarmRuntimeRestore(r.alarmId, false, acked);
    }
}
