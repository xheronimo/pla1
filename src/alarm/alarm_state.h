#pragma once
#include "alarm_struct.h"
#include <stdbool.h>

// Evalúa transición y genera evento si procede
bool alarmHandleStateChange(
    const AlarmRule& rule,
    bool active,
    float value
);
