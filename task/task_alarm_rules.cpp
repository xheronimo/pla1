#include "task/task_alarm_rules.h"

#include "alarm/alarm_struct.h"
#include "alarm/alarm_queue.h"
#include "alarm/alarm_rules.h"

#include "system/WatchdogManager.h"
#include "system/LogSystem.h"

#include <Arduino.h>

void taskAlarmRules(void*)
{
    escribirLog("ALM_RULES:START");

    for (;;)
    {
        watchdogKick(WDT_ALARM_RULES);

        auto& rules = alarmRulesGetAll();
        uint32_t now = millis();

        for (auto& r : rules)
        {
            if (!r.activa || !r.condicion)
                continue;

            bool cumple = r.condicion->evaluar();
            float valor = r.condicion->obtenerValorActual();

            // ---------- ACTIVACIÓN ----------
            if (cumple && !r.activaAhora)
            {
                if (r.tInicio == 0)
                    r.tInicio = now;

                if ((now - r.tInicio) >= r.retardoMs)
                {
                    r.activaAhora = true;
                    r.latched = true;

                    AlarmEvent ev{};
                    ev.grupo     = r.grupo;
                    ev.tipo      = ALM_RULE;
                    ev.estado    = ALARM_ON;
                    ev.id        = r.id;
                    ev.nombre    = r.id;
                    ev.mensaje   = r.mensaje;
                    ev.valor     = valor;
                    ev.timestamp = time(nullptr);

                    alarm_queuePush(ev);
                    escribirLog("ALM:ON:%s", r.id);
                }
            }

            // ---------- DESACTIVACIÓN ----------
            if (!cumple && r.activaAhora)
            {
                r.activaAhora = false;
                r.tInicio = 0;

                AlarmEvent ev{};
                ev.grupo     = r.grupo;
                ev.tipo      = ALM_RULE;
                ev.estado    = ALARM_OFF;
                ev.id        = r.id;
                ev.nombre    = r.id;
                ev.mensaje   = r.mensaje;
                ev.valor     = valor;
                ev.timestamp = time(nullptr);

                alarm_queuePush(ev);
                escribirLog("ALM:OFF:%s", r.id);
            }

            if (!cumple)
                r.tInicio = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
