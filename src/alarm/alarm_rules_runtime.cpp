#include "alarm_rules_runtime.h"
#include "signal/signal_manager.h"
#include "alarm/alarm_queue.h"
#include "alarm/alarm_struct.h"
#include "system/WatchdogManager.h"
#include "system/LogSystem.h"

extern AlarmRule alarmRules[];
extern uint8_t alarmRuleCount;

static bool evaluarCondicion(const AlarmCondition& c)
{
    float v;
    if (!getSignalValue(c.signalId, v))
        return false;

    switch (c.op) {
        case OP_EQ: return v == c.value;
        case OP_NE: return v != c.value;
        case OP_GT: return v >  c.value;
        case OP_LT: return v <  c.value;
        case OP_GE: return v >= c.value;
        case OP_LE: return v <= c.value;
    }
    return false;
}

void taskAlarmRules(void*)
{
    for (;;)
    {
        for (uint8_t i = 0; i < alarmRuleCount; i++)
        {
            AlarmRule& r = alarmRules[i];
            if (!r.activa) continue;

            bool estado = evaluarCondicion(r.cond);

            if (estado && !r.activaAhora)
            {
                r.activaAhora = true;
                r.latched = true;

                AlarmEvent ev{};
                ev.grupo  = r.grupos;
                ev.tipo   = ALM_RULE;
                ev.estado = ALARM_ON;
                ev.id     = r.id;
                ev.nombre = r.nombre;
                ev.mensaje= r.nombre;
                ev.valor  = 1;

                alarm_queuePush(ev);
            }

            if (!estado && r.activaAhora)
            {
                r.activaAhora = false;

                AlarmEvent ev{};
                ev.grupo  = r.grupos;
                ev.tipo   = ALM_RULE;
                ev.estado = ALARM_OFF;
                ev.id     = r.id;
                ev.nombre = r.nombre;
                ev.mensaje= r.nombre;
                ev.valor  = 0;

                alarm_queuePush(ev);
            }
        }

        watchdogKick(WDT_ALARM_RULES);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
