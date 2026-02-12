#include "alarm_utils.h"
#include <string.h>

const char* alarmSeverityToStr(AlarmSeverity s)
{
    switch (s)
    {
        case AlarmSeverity::INFO:     return "INFO";
        case AlarmSeverity::WARNING:  return "WARNING";
        case AlarmSeverity::CRITICAL: return "CRITICAL";
        default:                      return "UNKNOWN";
    }
}

const char* alarmGroupToStr(AlarmGroup g)
{
    switch (g)
    {
        case AlarmGroup::PROCESS: return "PROCESS";
        case AlarmGroup::SAFETY: return "SAFETY";
        case AlarmGroup::MAINTENANCE:  return "MAINTENANCE";
        case AlarmGroup::ENERGY:   return "ENERGY";
        case AlarmGroup::COMMUNICATION:   return "COMMUNICATION";
        default:                 return "UNKNOWN";
    }
}

AlarmSeverity parseAlarmSeverity(const char* s)
{
    if (!s) return AlarmSeverity::INFO;

    if (strcasecmp(s, "info") == 0)      return AlarmSeverity::INFO;
    if (strcasecmp(s, "warning") == 0)   return AlarmSeverity::WARNING;
    if (strcasecmp(s, "critical") == 0)  return AlarmSeverity::CRITICAL;

    // fallback seguro
    return AlarmSeverity::INFO;
}

AlarmGroup parseAlarmGroup(const char* s)
{
    if (!s) return AlarmGroup::PROCESS;

    if (strcasecmp(s, "process") == 0)        return AlarmGroup::PROCESS;
    if (strcasecmp(s, "safety") == 0)         return AlarmGroup::SAFETY;
    if (strcasecmp(s, "maintenance") == 0)    return AlarmGroup::MAINTENANCE;
    if (strcasecmp(s, "energy") == 0)         return AlarmGroup::ENERGY;
    if (strcasecmp(s, "communication") == 0)  return AlarmGroup::COMMUNICATION;

    // fallback seguro
    return AlarmGroup::PROCESS;
}
void freeExpr(AlarmExpr* e)
{
    if (!e) return;

    if (e->left)  freeExpr(e->left);
    if (e->right) freeExpr(e->right);
    if (e->child) freeExpr(e->child);

    delete e;
}



const char* alarmEventKindToStr(AlarmEventKind k)
{
    switch (k)
    {
        case AlarmEventKind::STATE_CHANGE: return "STATE_CHANGE";
        case AlarmEventKind::ACK:          return "ACK";
        default:                           return "UNKNOWN";
    }
}

