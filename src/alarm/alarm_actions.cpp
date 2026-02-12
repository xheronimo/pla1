// alarm_actions.cpp
#include "alarm_actions.h"
#include "alarm_utils.h"



static AlarmActions alarmActionsFor(AlarmSeverity s)
{
    switch (s)
    {
        case AlarmSeverity::INFO:
            return {
                true,   // mqtt
                true,   // web
                false,  // telegram
                false   // sms
            };

        case AlarmSeverity::WARNING:
            return {
                true,   // mqtt
                true,   // web
                true,   // telegram
                false   // sms
            };

        case AlarmSeverity::CRITICAL:
            return {
                true,   // mqtt
                true,   // web
                true,   // telegram
                true    // sms
            };
    }

    return { true, false, false, false };
}


