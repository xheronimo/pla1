#pragma once
#include <stdint.h>

// ===============================
// SENSOR ANALÓGICO
// ===============================
struct SensorAIConfig {
    bool activo;
    const char* id;
    const char* nombre;

    float rawMin;
    float rawMax;
    float realMin;
    float realMax;
};

// ===============================
// SENSOR DIGITAL
// ===============================
struct SensorDIConfig {
    bool activo;
    const char* id;
    const char* nombre;

    bool invertido;
};
