#pragma once
#include "alarm/alarm_struct.h"

struct PersistedAlarm {
    uint32_t alarmId;
    bool active;
    bool acked;
};


void alarmPersistenceLoad();
void alarmPersistenceSave(uint32_t alarmId, bool active, bool acked);
bool alarmPersistenceGet(uint32_t alarmId, bool& active, bool& acked);

void restoreAlarmRuntimeFromPersistence();

