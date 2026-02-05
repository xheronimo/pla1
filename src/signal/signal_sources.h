#pragma once
#include <stdint.h>

enum class SignalKind : uint8_t {
    DIGITAL = 0,
    ANALOG  = 1
};

enum class SignalBus : uint8_t {
    GPIO,
    I2C,
    MODBUS
};

struct SignalSource
{
    SignalKind kind;
    SignalBus  bus;

    // --- comunes ---
    bool invert = false;

    // --- GPIO ---
    uint8_t pin = 0;

    // --- I2C ---
    uint8_t i2cAddr = 0;
    uint8_t channel = 0;   // bit o canal

    // --- MODBUS ---
    uint8_t slave = 0;
    uint16_t address = 0;
    uint8_t count = 1;     // 1=bit, >1=registro
};
