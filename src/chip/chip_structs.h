#pragma once
#include <stdint.h>
#include <vector>
#include <string>
#include <Arduino.h>

// ==============================
// MODOS DE OPERACIÓN DEL CHIP
// ==============================
enum class ChipMode : uint8_t {
    INPUT_ONLY  = 0, // Solo sensores (Fallo no crítico para salidas)
    OUTPUT_ONLY = 1, // Solo actuadores (Fallo crítico)
    MIXED       = 2, // Sensores y actuadores (Fallo crítico)
    SYSTEM      = 3  // Chips internos (RTC, EEPROM)
};

enum class BusType : uint8_t {
    BUS_NONE = 0,
    BUS_GPIO,
    BUS_I2C,
    BUS_ONEWIRE,
    BUS_DHT11,
    BUS_DHT22,
    BUS_MODBUS, 
    BUS_VIRTUAL,
    BUS_MQTT,
    BUS_TELEGRAM,
    BUS_CAN
};

// ... (Enums Modbus: RegType, DataType, WordOrder se mantienen igual) ...

enum class ModbusRegType : uint8_t { MODBUS_COIL, MODBUS_DISCRETE_INPUT, MODBUS_HOLDING, MODBUS_INPUT };
enum class ModbusDataType : uint8_t { UINT16, INT16, UINT32, INT32, FLOAT32 };
enum class ModbusWordOrder : uint8_t { AB, BA, CDAB };

struct ChannelFlags {
    uint8_t used : 1;
    uint8_t valid : 1;
    uint8_t writable : 1;
    uint8_t inverted : 1;
    uint8_t reserved : 4;
};

union ChannelHardware {
    struct {
        uint16_t reg;
        uint8_t regType, dataType, wordOrder, wordCount;
    } mb;
    struct { uint8_t rom[8], busIdx; } ow;
    struct { uint8_t pin, mux; } io;
};

struct ChannelInfo {
    std::string signalId;
    ChannelFlags flags;
    ChannelHardware hw;
    float rawValue = 0.0f;
    uint32_t lastUpdateMs = 0;
};

// ==============================
// ESTRUCTURA CHIP FINAL
// ==============================
struct Chip {
    char id[32];
    char name[32];
    BusType bus;
    ChipMode mode;        // <--- Control de dirección de datos
    uint32_t deviceId; 
    bool systemReserved = false;
    std::vector<ChannelInfo> channels;
};