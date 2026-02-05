#pragma once

#include <stdint.h>

// ==================================================
// RUNTIME SENSOR
// ==================================================

struct SensorRuntime
{
    float ultimoValor = 0;
    bool  estado = false;

    uint32_t ultimoCambio = 0;
};
