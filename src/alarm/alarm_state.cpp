#include "alarm_state.h"

#include "alarm/alarm_runtime.h"
#include "alarm/alarm_queue.h"

bool alarmHandleStateChange(
    const AlarmRule& r,
    bool active,
    float value
)
{
    bool wasActive = alarmRuntimeIsActive(r.alarmId);

    // latch + ack → no generar evento
    if (r.latch && wasActive && alarmRuntimeIsAcked(r.alarmId))
        return false;

    if (active == wasActive)
        return false;

    alarmRuntimeSetActive(r.alarmId, active);

    AlarmEvent ev{};
    ev.alarmId = r.alarmId;
    ev.active  = active;
    ev.value   = value;
    ev.kind    = AlarmEventKind::STATE_CHANGE;

    alarm_queuePush(ev);
    return true;
}

