#include "alarm_rules.h"
#include "alarm_queue.h"
#include <Arduino.h>
#include <time.h>

static std::vector<AlarmRule> alarmRules;

// ==================================================
// EMITIR EVENTO DE REGLA
// ==================================================
static void sendRuleAlarm(const AlarmRule& r, bool recovery)
{
    AlarmEvent ev;

    ev.grupo  = (1ULL << r.grupo);
    ev.tipo   = ALM_RULE;
    ev.estado = recovery ? ALARM_OFF : ALARM_ON;

    ev.id      = r.id;
    ev.nombre  = r.id;
    ev.mensaje = r.mensaje;

    ev.valor     = NAN;
    ev.timestamp = time(nullptr);

    alarm_queuePush(ev);
}

// ==================================================
// UPDATE
// ==================================================
void alarmRulesUpdate()
{
    uint32_t now = millis();

    for (auto& r : alarmRules)
    {
        if (!r.activa || !r.condicion)
            continue;

        bool cumple = r.condicion->eval();

        if (cumple)
        {
            if (!r.activaAhora)
            {
                r.activaAhora = true;
                r.tInicio = now;
            }

            if (!r.latched &&
                (now - r.tInicio >= r.retardoMs))
            {
                sendRuleAlarm(r, false);
                r.latched = true;
            }
        }
        else
        {
            if (r.latched)
                sendRuleAlarm(r, true);

            r.activaAhora = false;
            r.latched = false;
        }
    }
}

// ==================================================
// INIT (vacío: reglas vienen de JSON)
// ==================================================
void alarmRulesInit()
{
    alarmRules.clear();
}

// ==================================================
// API
// ==================================================
void alarmRulesAdd(const AlarmRule& r)
{
    alarmRules.push_back(r);
}

void alarmRulesClear()
{
    alarmRules.clear();
}

void alarmRulesResetLatch(const char* ruleId)
{
    for (auto& r : reglas)
    {
        if (!strcmp(r.id, ruleId))
        {
            r.latched = false;
            escribirLog("ALM:LATCH_RESET:%s", r.id);
        }
    }
}
void alarmRulesForceLatch(const char* id)
{
    for (auto& r : reglas)
    {
        if (!strcmp(r.id, id))
        {
            r.latched = true;
            r.activaAhora = true;
        }
    }
}

void alarmRulesForEach(void (*cb)(const AlarmRule&))
{
    for (const auto& r : reglas)
        cb(r);
}
