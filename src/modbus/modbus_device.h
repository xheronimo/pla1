#pragma once
#include <stdint.h>
#include <string.h>

/**
 * @brief Estados de salud del dispositivo en el bus RS485
 */
enum class ModbusDeviceState : uint8_t { 
    STATE_OK = 0, 
    STATE_TIMEOUT, 
    STATE_FAULT,    
    STATE_OFFLINE   
};

/**
 * @brief Representación física de un esclavo Modbus
 */
struct ModbusDevice {
    uint8_t  id;             // Dirección Modbus (Slave ID)
    char     name[24];       // Nombre que debe coincidir con Chip.id para el vínculo
    uint16_t fingerprint;    // Identificador de perfil (0x0630, etc.)
    
    uint32_t baudrate;
    uint32_t pollMs;
    uint32_t lastPollMs;
    
    bool     enabled;
    ModbusDeviceState state;
    
    uint8_t  errorCount;
    uint32_t lastOkMs;
    
    bool     provisioning;   // ¿Está siendo configurado ahora mismo?
    bool     isDynamic;      // ¿Usa un perfil JSON o es Nativo?

    // Constructor con valores por defecto industriales
    ModbusDevice(uint8_t _id = 0, const char* _name = "", uint16_t _fp = 0) : 
        id(_id), fingerprint(_fp), baudrate(9600), pollMs(1000), 
        lastPollMs(0), enabled(true), state(ModbusDeviceState::STATE_OK), 
        errorCount(0), lastOkMs(0), provisioning(false), isDynamic(false) 
    {
        memset(name, 0, sizeof(name));
        if (_name) strncpy(name, _name, sizeof(name) - 1);
    }
};