#pragma once

#include <stdint.h>

// ==================================================
// TASK DE EVALUACIÓN DE REGLAS DE ALARMA
// ==================================================
//
// Evalúa periódicamente las reglas definidas
// y genera AlarmEvent ON / OFF que se envían
// a la cola de alarmas.
//
// Implementado en alarm_rules_runtime.cpp
//

#ifdef __cplusplus
extern "C" {
#endif

void taskAlarmRules(void*);

#ifdef __cplusplus
}
#endif
