#pragma once
#include <stdint.h>


// ==============================
// TIPO DE BUS FÍSICO
// ==============================
enum class BusType : uint8_t {
    BUS_GPIO = 0,
    BUS_PCF,
    BUS_I2C,
    BUS_MODBUS
};

// ==============================
// TIPO DE SEÑAL
// ==============================
enum class SignalKind {
    SENSOR_DIGITAL,
    SENSOR_ANALOG,
    ACTUATOR_DIGITAL
};
