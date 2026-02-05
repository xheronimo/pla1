#include "alarm_persistence.h"
#include "alarm/alarm_rules.h"
#include <LittleFS.h>

extern AlarmRule alarmRules[];
extern uint8_t alarmRuleCount;

void alarmPersistenceSave()
{
    File f = LittleFS.open("/alarm_state.dat", "w");
    if (!f) return;

    for (uint8_t i = 0; i < alarmRuleCount; i++)
        f.write((uint8_t*)&alarmRules[i].latched, sizeof(bool));

    f.close();
}

void alarmPersistenceLoad()
{
    File f = LittleFS.open("/alarm_state.dat", "r");
    if (!f) return;

    for (uint8_t i = 0; i < alarmRuleCount; i++)
        f.read((uint8_t*)&alarmRules[i].latched, sizeof(bool));

    f.close();
}
