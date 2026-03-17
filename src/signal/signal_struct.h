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
    BUS_VIRTUAL, 
    BUS_MQTT,     // Crucial para la comunicación entre placas
    BUS_TELEGRAM,
    BUS_CAN
};

enum class SignalMode : uint8_t {
    NORMAL,       // Sustituye a ACTIVE para ser más estándar
    TEST,         // Valor manual desde manualValue
    MANTENIMIENTO, // Valor congelado
    OFFLINE       // Señal ignorada por el sistema
};

enum class SignalQuality : uint8_t {
    GOOD = 0,
    ERROR = 1,
    CLAMPED = 2,
    INITIALIZING = 3,
    STALE = 4     // NUEVO: Para datos de MQTT que han llegado tarde (Latencia)
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
    char id[32];      // ID Técnico (ej: "AI_01")
    char name[32];    // Nombre amigable (ej: "Presion Tanque")
    
    BusType    bus = BusType::BUS_NONE;
    uint32_t   chipId = 0;  
    uint8_t    channel = 0; 
    SignalKind kind;
    SignalMode mode = SignalMode::NORMAL;
    
    float raw = 0.0f;         // Valor crudo del hardware
    float value = 0.0f;       // Valor procesado final
    float prev = 0.0f;        // NUEVO: Valor del ciclo anterior (para detectar cambios)
    float manualValue = 0.0f; // Para modo TEST
    
    bool writable = false;    // NUEVO: Indica si la señal acepta escrituras (Actuadores/MQTT)
    bool valid = false;       // Indica si el último poll fue exitoso
    
    SignalCalib   calib;
    SignalQuality quality = SignalQuality::INITIALIZING;
    
    // Lógica Virtual
    VirtualOp virtualOp = VirtualOp::NONE;
    char sourceA[32]; 
    char sourceB[32]; 

    bool invertido = false;
    bool systemReserved = false;
    
    uint32_t lastUpdateMs = 0; // Timestamp local para Watchdog de red
};