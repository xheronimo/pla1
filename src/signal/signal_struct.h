#pragma once
#include <stdint.h>
#include "bus/bus_struct.h"
#include "i2c/i2c_chip.h"

struct Calibration
{
    float rawMin;
    float rawMax;
    float realMin;
    float realMax;
    bool clamp;
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
    BA  // swapped
};

struct Signal
{
    const char *id;

    SignalKind kind; // DIGITAL / ANALOG
    BusType bus;     // GPIO / PCF / I2C / MODBUS

    uint8_t address; // I2C / PCF / Modbus slave
    uint8_t channel; // pin / bit / register
    uint8_t wordCount;
    bool signedValue; // int / uint
    ModbusDataType dataType;
    ModbusWordOrder wordOrder;
    bool writable;   // 👈 CLAVE
    // 🔽 SOLO PARA MODBUS

    ModbusRegType modbusType;

    I2CDevice chip;      // 👈 clave

    bool invertido; // SOLO DIGITAL

    // --- valores ---
    float raw;   // valor leído (tras debounce / invertido)
    float prev;  // valor previo (para trigger)
    float value; // valor calibrado (ANALOG)

    Calibration calib; // SOLO ANALOG
    bool error;

    // --- debounce (SOLO DIGITAL) ---
    uint16_t debounceMs;
    float stableValue;
    uint32_t lastChangeTs;
    bool initialized; // 👈 NUEVO

    uint8_t errorCount; // 👈 importante
    uint32_t lastOkTs;  // 👈 watchdog
};
