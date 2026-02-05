#pragma once
#include <stdint.h>

enum AlarmTriggerType : uint8_t {
    ALM_MIN,
    ALM_MAX,
    ALM_ERR,
    ALM_DIG,
    ALM_RULE,
    ALM_SYS
};

enum AlarmEventState : uint8_t {
    ALARM_OFF = 0,
    ALARM_ON  = 1
};

struct AlarmEvent {
    uint64_t grupo;
    AlarmTriggerType tipo;
    AlarmEventState estado;

    const char* id;
    const char* nombre;
    const char* mensaje;

    float valor;
    uint32_t timestamp;
};

struct AlarmRule {
    const char* id;
    const char* mensaje;

    bool activa = true;
    uint64_t grupo = 0;
    uint32_t retardoMs = 0;

    // runtime
    bool activaAhora = false;
    bool latched = false;
    uint32_t tInicio = 0;
};
