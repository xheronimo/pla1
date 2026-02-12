#include "alarm_runtime.h"
#include "alarm_struct.h"
#include "alarm_queue.h"
#include "alarm_persistence.h"


#define MAX_ALARM_RUNTIME 32

static AlarmRuntime table[MAX_ALARM_RUNTIME];
static uint8_t count = 0;

const AlarmRuntime* alarmRuntimeAll(size_t& outCount)
{
    outCount = count;
    return table;
}


// --------------------------------------------------
static AlarmRuntime* find(uint32_t alarmId)
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
    AlarmRuntime* r = alarmRuntimeGet(alarmId);
    if (!r) return;

    if (!r)
    {
        if (count >= MAX_ALARM_RUNTIME) return;
        r = &table[count++];
        r->alarmId = alarmId;
        r->acked   = false;
        r->active  = false;
    }

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
    if (!r) {
        if (count >= MAX_ALARM_RUNTIME) return;
        r = &table[count++];
        r->alarmId = alarmId;
    }

    r->active = active;
    r->acked  = acked;
}


// --------------------------------------------------
bool alarmRuntimeIsActive(uint32_t alarmId)
{
    AlarmRuntime* r = find(alarmId);
    return r ? r->active : false;
}

// --------------------------------------------------
bool alarmRuntimeIsAcked(uint32_t alarmId)
{
    AlarmRuntime* r = find(alarmId);
    return r ? r->acked : false;
}

// --------------------------------------------------


void alarmRuntimeAck(uint32_t alarmId)
{
    AlarmRuntime* r = find(alarmId);
    if (!r || r->acked) return;

    r->acked = true;

    // Persistir
    alarmPersistenceSave(alarmId, r->active, r->acked);

    // Emitir evento ACK
    AlarmEvent ev{};
    ev.alarmId = alarmId;
    ev.active  = r->active;
    ev.kind    = AlarmEventKind::ACK;

    alarm_queuePush(ev);
}
void alarmRuntimeClearAll()
{
    count = 0;
}

AlarmRuntime* alarmRuntimeGet(uint32_t alarmId)
{
    AlarmRuntime* r = find(alarmId);
    if (r) return r;

    if (count >= MAX_ALARM_RUNTIME)
        return nullptr;

    r = &table[count++];
    r->alarmId = alarmId;
    r->active  = false;
    r->acked   = false;
    return r;
}