#include "alarm_runtime.h"
#include "alarm_persistence.h"

void alarmAck(uint32_t alarmId)
{
    alarmRuntimeAck(alarmId);

    bool active = false;
    bool acked  = false;

    AlarmRuntime* r = alarmRuntimeGet(alarmId);
    if (!r) return;

    alarmPersistenceSave(alarmId, r->active, r->acked);
}
