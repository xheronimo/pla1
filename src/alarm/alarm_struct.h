#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <vector>
#include "alarm_expr.h"

// ==============================
// ENUMERACIONES DE ESTADO
// ==============================
enum class AlarmSeverity : uint8_t {
    INFO = 0,
    WARNING,
    MAINTENANCE, // Añadido MAINTENANCE como pediste
    CRITICAL
};

enum class AlarmGroup : uint8_t {
    PROCESS,
    SAFETY,
    MAINTENANCE,
    ENERGY,
    COMMUNICATION
};

enum class AlarmEventKind : uint8_t {
    STATE_CHANGE,   // ON / OFF
    ACK             // Reconocimiento manual
};

// ===================================
// REGLA DE ALARMA (DEFINICIÓN)
// ===================================
struct AlarmRule {
    uint32_t alarmId;
    std::string name;          // Nombre descriptivo de la alarma
    AlarmExpr* expr;           // Árbol lógico/analógico de evaluación
    
    bool latch;                // Si es true, requiere ACK para desactivarse
    AlarmGroup group;          // Área a la que pertenece
    AlarmSeverity severity;    // Nivel de criticidad

    // --- ACCIÓN FÍSICA (INTERLOCK) ---
    // Usamos char[16] o std::string según prefieras para el ChipID
    char actionChipId[16];     
    uint8_t actionChannel;     
    float actionValue;         
    bool hasAction;            // ¿Dispara hardware al activarse?

    // --- CANALES DE NOTIFICACIÓN ---
    bool sendMqtt;
    bool sendWeb;
    bool sendTelegram;
    bool sendSms;

    // --- ESTADO EN TIEMPO REAL (Para evitar bucles externos) ---
    bool isActive;             // Estado actual
    bool isAcknowledged;       // Si el operador ya la vió
    uint32_t lastTriggerMs;    // Timestamp del último disparo
};

// ===================================
// EVENTO DE ALARMA (PARA LOGS/SD)
// ===================================
struct AlarmEvent {
    uint32_t timestamp;        // Segundos EPOCH
    uint32_t alarmId;
    bool active;               // true = activada, false = recuperada
    AlarmEventKind kind;       
    float valueAtTrigger;      // Valor de la señal en el momento del fallo
    char signalId[32];         // ID de la señal que causó el disparo
};