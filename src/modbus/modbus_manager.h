#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include <Arduino.h>
#include <ModbusRTU.h>
#include <vector>

struct ModbusGroup {
    uint8_t slaveId;
    uint16_t startReg;
    uint16_t count;
    uint16_t* buffer;
    uint8_t errorCount;
    bool isAlive;
    std::vector<String> signalIds; 
};

class ModbusManager {
public:
    static void init(HardwareSerial& serial, uint32_t baud, int8_t txPin = -1);
    
    // El motor que llama tu main.cpp
    static void pollAll(); 
    
    static void updateWatchdog();
    static void autoBuildGroups();
    static void addGroup(uint8_t slave, uint16_t start, uint16_t count, std::vector<String> ids);

private:
    // Decodificador de tipos industriales
    static float decodeValue(uint16_t* regs, SignalKind kind, bool swapped);
    static void reportSuccess(uint8_t slaveId);
    static void reportError(uint8_t slaveId);

    static ModbusRTU _mb;
    static std::vector<ModbusGroup> _groups;
    static unsigned long _lastPollTime;
    static const uint8_t MAX_ERRORS = 5;
};

#endif