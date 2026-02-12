#include "alarm/alarm_config.h"

#define MAX_ALARM_RULES 32

static AlarmRule alarmRules[MAX_ALARM_RULES];
static size_t alarmRuleCount = 0;

// --------------------------------------------------
// API pública
// --------------------------------------------------
void alarmGetRules(const AlarmRule*& rules, size_t& count)
{
    rules = alarmRules;
    count = alarmRuleCount;
}

// --------------------------------------------------
// Usada por el loader de configuración
// --------------------------------------------------
bool alarmAddRule(const AlarmRule& r)
{
    if (alarmRuleCount >= MAX_ALARM_RULES)
        return false;

    alarmRules[alarmRuleCount++] = r;
    return true;
}
