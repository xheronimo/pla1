#pragma once
#include <stdint.h>

void alarmPersistenceSave(uint32_t alarmId, bool active, bool acked);

void alarmPersistenceLoadAll();
