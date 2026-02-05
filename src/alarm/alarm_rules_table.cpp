#include "alarm/alarm_rules.h"

AlarmRule alarmRules[] = {
    {
        "R1",
        "Regla prueba",
        true,
        1,
        {
            "S1",
            OP_GT,
            0.5f
        },
        false,
        false
    }
};

uint8_t alarmRuleCount = sizeof(alarmRules) / sizeof(AlarmRule);
