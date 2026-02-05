#pragma once
#include "sensor_types.h"

struct SensorSource {
    SensorSourceType type;
    SensorKind kind;

    // Comunes
    uint8_t address;   // I2C / PCF / Modbus slave
    uint16_t channel;  // pin / bit / registro

    // Opcional
    bool invertido;
};

struct Sensor {
    const char* id;
    const char* nombre;

    SensorSource src;

    // Runtime
    float valor;
    bool estado;          // para digitales
};
