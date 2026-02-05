#include "sensor_table.h"

// ==================================================
// EJEMPLO (luego vendrá de JSON)
// ==================================================

SensorConfig sensores[] =
{
    // Digital GPIO
    { "D1", SensorKind::DIGITAL, SensorSourceType::GPIO,  25, 0, 0, false },

    // Digital I2C (PCF)
    { "D2", SensorKind::DIGITAL, SensorSourceType::I2C,   4,  0x20, 0, false },

    // Analógico Modbus
    { "A1", SensorKind::ANALOG,  SensorSourceType::MODBUS,0,  1, 30001, false, 0, 1.0f }
};

const int NUM_SENSORES = sizeof(sensores) / sizeof(SensorConfig);