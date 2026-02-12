#include "alarm_persistence.h"
#include "alarm_registry.h"
#include "alarm_runtime.h"

#define MAX_PERSISTED_ALARMS 16

// ⚠️ Implementación temporal en RAM
// En el futuro esto se sustituye por NVS / Flash
static PersistedAlarm table[MAX_PERSISTED_ALARMS];
static uint8_t count = 0;

// --------------------------------------------------
// Cargar (stub por ahora)
// --------------------------------------------------
void alarmPersistenceLoad()
{
    count = 0;
    // FUTURO:
    // - Abrir NVS
    // - Leer lista de alarmas
    // - Rellenar table[]
}

// --------------------------------------------------
// Guardar estado de una alarma
// --------------------------------------------------
void alarmPersistenceSave(uint32_t alarmId, bool active, bool acked)
{
    for (uint8_t i = 0; i < count; i++)
    {
        if (table[i].alarmId == alarmId)
        {
            table[i].active = active;
            table[i].acked = acked;
            return;
        }
    }

    if (count < MAX_PERSISTED_ALARMS)
    {
        table[count].alarmId = alarmId;
        table[count].active = active;
        table[count].acked = acked;
        count++;
    }
    // FUTURO:
    // - Guardar inmediatamente en NVS
}

// --------------------------------------------------
// Obtener estado persistido
// --------------------------------------------------
bool alarmPersistenceGet(uint32_t alarmId, bool &active, bool &acked)
{
    for (uint8_t i = 0; i < count; i++)
    {
        if (table[i].alarmId == alarmId)
        {
            active = table[i].active;
            acked = table[i].acked;
            return true;
        }
    }
    return false;
}

void restoreAlarmRuntimeFromPersistence()
{
    size_t count = 0;
    const AlarmRule *rules = alarmRegistryAll(count);

    for (size_t i = 0; i < count; i++)
    {
        bool active, acked;
        if (alarmPersistenceGet(rules[i].alarmId, active, acked))
        {
            alarmRuntimeRestore(rules[i].alarmId, active, acked);
        }
    }
}
