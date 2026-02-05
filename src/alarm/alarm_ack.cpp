#include "alarm_ack.h"
#include "alarm/alarm_rules.h"
#include <string.h>

extern AlarmRule alarmRules[];
extern uint8_t alarmRuleCount;

void alarmAckById(const char* id)
{
    for (uint8_t i = 0; i < alarmRuleCount; i++)
    {
        if (strcmp(alarmRules[i].id, id) == 0)
        {
            alarmRules[i].latched = false;
        }
    }
}

bool alarmIsAcked(const char* id)
{
    for (uint8_t i = 0; i < alarmRuleCount; i++)
    {
        if (strcmp(alarmRules[i].id, id) == 0)
            return !alarmRules[i].latched;
    }
    return false;
}

void alarmClearAck(const char* id)
{
    for (uint8_t i = 0; i < alarmRuleCount; i++)
    {
        if (strcmp(alarmRules[i].id, id) == 0)
            alarmRules[i].latched = true;
    }
}
