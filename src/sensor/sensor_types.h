#pragma once
#include <stdint.h>

enum SensorKind : uint8_t {
    SENSOR_DIGITAL = 0,
    SENSOR_ANALOG  = 1
};

enum SensorSourceType : uint8_t {
    SRC_GPIO = 0,
    SRC_PCF,
    SRC_I2C,
    SRC_MODBUS
};
