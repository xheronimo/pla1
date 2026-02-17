#include "alarm_runtime.h"
#include "alarm_struct.h"
#include "alarm_queue.h"
#include "alarm_persistence.h"
#include "system/nvs_store.h"

#define MAX_ALARM_RUNTIME 32

static AlarmRuntime table[MAX_ALARM_RUNTIME];
static uint8_t count = 0;

const AlarmRuntime *alarmRuntimeAll(size_t &outCount)
{
    outCount = count;
    return table;
}

// --------------------------------------------------
static AlarmRuntime *find(uint32_t alarmId)
{
    for (uint8_t i = 0; i < count; i++)
    {
        if (table[i].alarmId == alarmId)
            return &table[i];
    }
    return nullptr;
}

// --------------------------------------------------
void alarmRuntimeSetActive(uint32_t alarmId, bool active)
{
    AlarmRuntime *r = alarmRuntimeGet(alarmId);
    if (!r)
        return;

    if (r->blocked)
        return;

    // Cambio de estado
    if (r->active != active)
    {
        r->active = active;
        if (active)
            r->acked = false; // nueva activación → no reconocida
    }
}

void alarmRuntimeRestore(uint32_t alarmId, bool active, bool acked)
{
    AlarmRuntime* r = find(alarmId);
    if (!r)
                return;
    

    r->active = active;
    r->acked  = acked;
}



// --------------------------------------------------
bool alarmRuntimeIsActive(uint32_t alarmId)
{
    AlarmRuntime *r = find(alarmId);
    return r ? r->active : false;
}

// --------------------------------------------------
bool alarmRuntimeIsAcked(uint32_t alarmId)
{
    AlarmRuntime *r = find(alarmId);
    return r ? r->acked : false;
}

// --------------------------------------------------

void alarmRuntimeAck(uint32_t alarmId)
{
    AlarmRuntime* r = find(alarmId);
    if (!r || r->acked)
        return;

    r->acked = true;

    auto& nvs = NVS::alarms();
    char key[24];
    snprintf(key, sizeof(key), "ack_%lu", alarmId);
    nvs.putBool(key, true);
}



void alarmRuntimeClearAll()
{
    count = 0;
}

AlarmRuntime *alarmRuntimeGet(uint32_t alarmId)
{
    AlarmRuntime *r = find(alarmId);
    if (r)
        return r;

    if (count >= MAX_ALARM_RUNTIME)
        return nullptr;

    r = &table[count++];
    r->alarmId = alarmId;
    r->active = false;
    r->acked = false;
    r->blocked = false; 
    return r;
}

// --------------------------------------------------
void alarmRuntimeSetBlocked(uint32_t alarmId, bool blocked)
{
    AlarmRuntime *r = alarmRuntimeGet(alarmId);
    if (!r)
        return;

    r->blocked = blocked;

    // Política SAFE: si está bloqueada, nunca puede estar activa
    if (blocked)
        r->active = false;
}

// --------------------------------------------------
bool alarmRuntimeIsBlocked(uint32_t alarmId)
{
    AlarmRuntime *r = find(alarmId);
    if (!r)
        return true; // SAFE por defecto

    return r->blocked;
}


