#pragma once
#include <stdint.h>
#include <stdbool.h>

// Operaciones para señales calculadas
enum class VirtualOp : uint8_t {
    NONE = 0,
    SUM,    // A + B
    DIFF,   // A - B
    AND,    // A && B (Digital)
    OR,     // A || B (Digital)
    MUL     // A * B
};

enum class SignalKind : uint8_t {
    SENSOR_DIGITAL,
    SENSOR_ANALOG,
    ACTUATOR_DIGITAL,
    ACTUATOR_ANALOG
};

enum class BusType : uint8_t {
    BUS_NONE = 0,
    BUS_GPIO,
    BUS_I2C,
    BUS_ONEWIRE,
    BUS_DHT11,
    BUS_DHT22,
    BUS_MODBUS,
    BUS_VIRTUAL, // Importante para el Manager
    BUS_MQTT,
    BUS_TELEGRAM,
    BUS_CAN
};

enum class SignalMode : uint8_t {
    ACTIVE,
    TEST,
    MANTENIMIENTO, // Valor congelado para reparaciones
    OFFLINE
};

enum class SignalQuality : uint8_t {
    GOOD,
    ERROR,
    CLAMPED,
    INITIALIZING
};

struct SignalCalib {
    bool     usaescala = false;
    float    rawMin = 0.0f, rawMax = 4095.0f;
    float    realMin = 0.0f, realMax = 100.0f;
    float    offset = 0.0f;
    bool     clamp = true;
    float    emaAlpha = 1.0f;
    float    emaValue = 0.0f;
    bool     emaInit = false;
    float    measureHysteresis = 0.01f;
    float    lastStableValue = 0.0f;
};

struct Signal {
    char id[32];
    char name[32];
    
    BusType    bus;
    uint32_t   chipId;  
    uint8_t    channel; 
    SignalKind kind;
    SignalMode mode = SignalMode::ACTIVE;
    
    float raw = 0.0f;
    float value = 0.0f;
    float manualValue = 0.0f; // Para modo TEST
    
    SignalCalib   calib;
    SignalQuality quality = SignalQuality::INITIALIZING;
    
    // Lógica Virtual
    VirtualOp virtualOp = VirtualOp::NONE;
    char sourceA[32]; // ID de la señal A
    char sourceB[32]; // ID de la señal B

    bool invertido = false;
    bool systemReserved = false;
    
    uint32_t lastUpdateMs = 0;
};