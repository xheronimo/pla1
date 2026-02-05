#include "alarm_dispatcher.h"

#include "alarm/alarm_struct.h"
#include "alarm/alarm_router.h"
#include "alarm/alarm_queue.h"

#include "system/WatchdogManager.h"
#include "system/LogSystem.h"

#include <Arduino.h>

// ==================================================
// TASK DISPATCHER DE ALARMAS
// ==================================================

void taskAlarmDispatcher(void*)
{
    AlarmEvent ev;

    for (;;)
    {
        if (alarm_queuePop(ev))
        {
            bool enviado = AlarmRouter::emitirEvento(ev);

            if (!enviado)
            {
                escribirLog("ALM:REQUEUE");
                vTaskDelay(pdMS_TO_TICKS(3000));
                alarm_queuePush(ev);
            }
        }

        watchdogKick(WDT_ALARM_DISPATCHER);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
