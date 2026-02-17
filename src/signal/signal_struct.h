#pragma once
#include <stdint.h>
#include "bus/bus_struct.h"
#include "i2c/i2c_chip.h"


struct Calibration
{
    float rawMin = 0;
    float rawMax = 0;
    float realMin = 0;
    float realMax = 0;
    float offset = 0;

    bool  clamp = false;

    float measureHysteresis = 0;
    bool  hasStableValue = false;
    float lastStableValue = 0;

    float emaAlpha = 0;
    bool  emaInit = false;
    float emaValue = 0;

};

enum class ModbusRegType : uint8_t
{
    MODBUS_COIL,
    MODBUS_DISCRETE_INPUT,
    MODBUS_HOLDING,
    MODBUS_INPUT
};

enum class ModbusDataType : uint8_t
{
    UINT16,
    INT16,
    UINT32,
    INT32,
    FLOAT32
};

enum class ModbusWordOrder : uint8_t
{
    AB, // normal (reg, reg+1)
    BA, // swapped
    CDAB
};

enum class SignalQuality : uint8_t
{
    GOOD,
    UNSTABLE,
    CLAMPED,
    ERROR
};



struct Signal
{
    const char *id;

    SignalKind kind; // DIGITAL / ANALOG
    BusType bus;     // GPIO / PCF / I2C / MODBUS

    uint8_t address; // I2C / PCF / Modbus slave
    uint8_t channel; // pin / bit / register
    uint8_t wordCount;
    bool signedValue;

    bool writable;

    I2CDevice chip;
    uint32_t options;

    bool systemReserved = false;
    bool lockedConfig = false;

    bool invertido;

    // --- valores ---
    float raw;
    float prev;
    float value;

    Calibration calib;

    bool error = false;

    // --- debounce ---
    uint16_t debounceMs = 0;
    float stableValue = 0;
    uint32_t lastChangeTs = 0;
    bool initialized = false;

    uint8_t errorCount = 0;
    uint32_t lastOkTs = 0; // ← SOLO UNA VEZ

    bool valid = false;

    // MODBUS
    ModbusDataType dataType;
    ModbusWordOrder wordOrder;
    ModbusRegType modbusType;

    SignalQuality quality = SignalQuality::GOOD;
    uint32_t lastUpdateMs ;
};
