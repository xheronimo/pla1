#include "alarm_struct.h"

const char* alarmTriggerTypeToString(AlarmTriggerType t)
{
    switch (t)
    {
        case ALM_MIN:  return "MIN";
        case ALM_MAX:  return "MAX";
        case ALM_ERR:  return "ERROR";
        case ALM_DIG:  return "DIGITAL";
        case ALM_RULE: return "RULE";
        case ALM_SYS:  return "SYSTEM";
        default:       return "UNKNOWN";
    }
}
