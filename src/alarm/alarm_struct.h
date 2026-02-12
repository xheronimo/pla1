#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "alarm_expr.h"


// ==============================
// TIPO DE CONDICIÓN DE ALARMA
// ==============================
enum class AlarmCondition : uint8_t {
    MIN,        // valor < umbral
    MAX,        // valor > umbral
    TRIGGER,    // flanco / digital ON
    ERROR       // error del sensor
};

enum class TriggerMode : uint8_t {
    LEVEL = 0,     // mientras esté activo
    RISE,    // flanco subida
    FALL    // flanco bajada
};



enum class AlarmEventKind : uint8_t {
    STATE_CHANGE,   // ON / OFF
    ACK             // ACK manual
};

// --------------------
// Severidad de alarma
// --------------------
enum class AlarmSeverity : uint8_t {
    INFO = 0,
    WARNING,
    CRITICAL
};

enum class AlarmGroup : uint8_t {
    PROCESS,
    SAFETY,
    MAINTENANCE,
    ENERGY,
    COMMUNICATION
};



// ==============================
// EVENTO DE ALARMA (RUNTIME)
// ==============================
struct AlarmEvent {
    uint32_t alarmId;

    bool active;                // ON / OFF
    AlarmEventKind kind;        // STATE_CHANGE / ACK / CLEAR / RECOVER

    // Contexto (opcional, para UI / logs)
    uint32_t primarySignalId;   // 0 si no aplica
    float    value;             // valor asociado (si aplica)
};




//REGLAS ED ALARMAS 
struct AlarmRule {
    uint32_t alarmId;
    AlarmExpr* expr;   // 👈 aquí está la magia
    bool latch;
      AlarmGroup    group;      // 👉 a qué área pertenece
    AlarmSeverity severity;     // INFO / WARNING / MAINT / CRITICAL

    bool sendMqtt;
    bool sendWeb;
    bool sendTelegram;
    bool sendSms;
};
