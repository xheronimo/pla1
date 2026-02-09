#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "signal/signal_struct.h"

// -----------------------------
// REGISTROS INA219
// -----------------------------
#define INA219_REG_CONFIG        0x00
#define INA219_REG_SHUNT_VOLT    0x01
#define INA219_REG_BUS_VOLT      0x02
#define INA219_REG_POWER         0x03
#define INA219_REG_CURRENT       0x04
#define INA219_REG_CALIBRATION   0x05

// -----------------------------
// AJUSTES HW
// -----------------------------
constexpr float INA219_SHUNT_OHMS = 0.1f;

// -----------------------------
// CACHE
// -----------------------------
#define INA219_MAX_CHIPS   8
#define INA219_CACHE_MS    500

struct Ina219Cache {
    bool     valid;
    uint32_t lastReadMs;
    float    voltage;   // V
    float    current;   // A
    float    power;     // W
};

// -----------------------------
// API
// -----------------------------
bool leerSignalINA219(const Signal& s, float& out, uint8_t options);
void ina219ResetCaches();
