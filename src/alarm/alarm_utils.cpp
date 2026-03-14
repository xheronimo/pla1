#include "alarm/alarm_struct.h"
#include <string.h>

// Helper para liberar la memoria del árbol de expresiones (Recursivo)
// Se usa en AlarmMgr::clearRules()
void freeExpr(AlarmExpr* e) {
    if (!e) return;
    if (e->left)  freeExpr(e->left);
    if (e->right) freeExpr(e->right);
    if (e->child) freeExpr(e->child);
    delete e;
}

const char* alarmSeverityToStr(AlarmSeverity s) {
    switch (s) {
        case AlarmSeverity::INFO:     return "INFO";
        case AlarmSeverity::WARNING:  return "WARNING";
        case AlarmSeverity::CRITICAL: return "CRITICAL";
        default:                      return "UNKNOWN";
    }
}

const char* alarmGroupToStr(AlarmGroup g) {
    switch (g) {
        case AlarmGroup::PROCESS:       return "PROCESS";
        case AlarmGroup::SAFETY:        return "SAFETY";
        case AlarmGroup::MAINTENANCE:   return "MAINTENANCE";
        case AlarmGroup::ENERGY:        return "ENERGY";
        case AlarmGroup::COMMUNICATION: return "COMMUNICATION";
        default:                        return "UNKNOWN";
    }
}

// Estos "parsers" son los que usa el alarm_json_loader.cpp
AlarmSeverity parseAlarmSeverity(const char* s) {
    if (!s) return AlarmSeverity::INFO;
    if (strcasecmp(s, "critical") == 0) return AlarmSeverity::CRITICAL;
    if (strcasecmp(s, "warning") == 0)  return AlarmSeverity::WARNING;
    return AlarmSeverity::INFO;
}