#pragma once

#include <stdint.h>

// =======================================================
// GRUPOS DE ALARMA (bitmask)
// =======================================================

typedef uint64_t AlarmGroupMask;

// =======================================================
// TIPO DE DISPARO
// =======================================================

enum AlarmTriggerType : uint8_t {
    ALM_MIN = 0,
    ALM_MAX,
    ALM_ERR,
    ALM_DIG,
    ALM_RULE,   // reglas lógicas
    ALM_SYS     // sistema / watchdog / firmware
};

const char* alarmTypeToString(AlarmTriggerType t);
// =======================================================
// ESTADO DEL EVENTO
// =======================================================

enum AlarmEventState : uint8_t {
    ALARM_OFF = 0,   // recuperación
    ALARM_ON  = 1    // disparo
};

// =======================================================
// EVENTO DE ALARMA (RUNTIME)
// =======================================================
// 👉 Esto es lo que viaja por la cola, router, MQTT, SMS…

struct AlarmEvent {

    // --- clasificación ---
    AlarmGroupMask   grupo;      // bitmask de grupos
    AlarmTriggerType tipo;       // MIN / MAX / ERR / DIG / RULE / SYS
    AlarmEventState  estado;     // ON / OFF

    // --- identidad ---
    const char* id;              // ID técnico (P1, V2, D3, A01…)
    const char* nombre;          // Nombre humano (Presión 1, Motor…)
    const char* mensaje;         // Mensaje configurado

    // --- contexto ---
    float     valor;             // valor que disparó
    uint32_t  timestamp;         // epoch seconds
};
const char* alarmTriggerTypeToString(AlarmTriggerType t);