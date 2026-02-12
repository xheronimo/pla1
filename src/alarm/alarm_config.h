#pragma once
#include <stddef.h>
#include "alarm/alarm_struct.h"

// Devuelve puntero + número de reglas
void alarmGetRules(const AlarmRule*& rules, size_t& count);
