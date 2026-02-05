#include "task_alarm_dispatcher.h"

#include <Arduino.h>

#include "alarm/alarm_queue.h"
#include "alarm/alarm_router.h"
#include "system/WatchdogManager.h"
#include "system/LogSystem.h"

// ==================================================
// TASK DISPATCHER
// ==================================================
void taskAlarmDispatcher(void*)
{
    AlarmEvent ev;

    for (;;)
    {
        // Esperar evento (bloqueante)
        if (!alarm_queuePop(ev))
        {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        bool enviado = false;

        // ----------------------------------
        // ROUTER (SMS / MQTT / Telegram)
        // ----------------------------------
        enviado = AlarmRouter::emitirEvento(ev);

        // ----------------------------------
        // FALLO TOTAL → REENCOLAR
        // ----------------------------------
        if (!enviado)
        {
            escribirLog("ALM:REQUEUE");

            vTaskDelay(pdMS_TO_TICKS(3000));

            alarm_queuePush(ev);
        }

        watchdogKick(WDT_ALARM_DISPATCH);
    }
}
