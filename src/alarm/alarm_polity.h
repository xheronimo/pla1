#pragma once

#include "alarm/alarm_struct.h"
#include "alarm/alarm_destination.h"

class AlarmPolicyEngine {
public:
    static bool shouldSend(
        const AlarmEvent&,
        const AlarmDestination&
    )
    {
        return true; // TODO política real
    }
};
