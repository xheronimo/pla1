
#pragma once

#include "alarm/alarm_struct.h"
#include "alarm/alarm_destination.h"

class AlarmPolicyEngine {
public:
    static bool permitirEnvio(
        const AlarmEvent&,
        const AlarmDestination&
    ) {
        return true; // stub
    }
};
