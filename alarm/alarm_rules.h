#pragma once
#include <vector>
#include <stdint.h>
#include "alarm_struct.h"
#include "alarm_expr.h"

// ==================================================
// REGLA DE ALARMA
// ==================================================
struct AlarmRule
{
    const char* id;
    const char* mensaje;

    bool activa;
    uint64_t grupo;
    uint32_t retardoMs;

    AlarmExpr* condicion;

    // --- runtime ---
    bool activaAhora = false;
    bool latched = false;
    uint32_t tInicio = 0;
};



const std::vector<AlarmRule>& alarmRulesGetAll();


// ==================================================
// API
// ==================================================
void alarmRulesInit();
void alarmRulesUpdate();

void alarmRulesAdd(const AlarmRule& r);
void alarmRulesClear();
void alarmRulesResetLatch(const char* ruleId);
void alarmRulesForceLatch(const char* id);
void alarmRulesForEach(void (*cb)(const AlarmRule&));
