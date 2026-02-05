#pragma once
#include "alarm_runtime.h"
#include "alarm/alarm_struct.h"

class AlarmPolicy
{
public:
    static bool permitirEnvio(
        const AlarmEvent& ev,
        AlarmRuntime& rt,
        uint32_t minIntervalMs
    );
};
