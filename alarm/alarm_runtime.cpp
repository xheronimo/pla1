
// alarm_rule_runtime.h
#pragma once
#include <stdint.h>

struct AlarmRuleRuntime
{
    bool activo = false;        // condición verdadera AHORA
    bool latched = false;       // alarma enganchada
    bool pendiente = false;    // esperando retardo

    uint32_t tInicio = 0;       // inicio del retardo
    float ultimoValor = 0;      // último valor evaluado
};



struct AlarmRuntime
{
    uint32_t ultimoEnvioMs = 0;
    float    ultimoValor   = 0;
    bool     ultimoEstado  = false;
};
