#include "task/task_alarm_dispatcher.h"
#include "alarm/alarm_queue.h"
#include "alarm/alarm_router.h"

void taskAlarmDispatcher(void* arg)
{
    AlarmRouter router;
    AlarmEvent ev;

    while (true)
    {
        if (alarm_queuePop(ev))   // ← SOLO un parámetro
        {
            router.emitirEvento(ev);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
