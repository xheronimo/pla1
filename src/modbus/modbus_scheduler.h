#pragma once

#include <vector>
#include <stdint.h>
#include "modbus_device.h" 

namespace ModbusScheduler {

    // --- Ciclo de Vida ---
    void init(size_t reservedDevices = 16);
    bool addDevice(const ModbusDevice& dev);
    void clearDevices();

    // --- Ejecución ---
    void pollAll();          // El motor de polling
    void startTask(uint16_t stackSize = 4096, uint8_t priority = 2);

    // --- Métricas (Sustituye a modbus_metrics.h) ---
    uint32_t getTotalErrors();   // Suma de errores de todos los dispositivos
    uint32_t getTotalReads();    // Suma de lecturas exitosas
    float getBusLoad();          // % de tiempo que el bus está ocupado
    const char* getBusStatus();  // "OK", "DEGRADED", "DOWN"

    // --- Búsqueda ---
    ModbusDevice* getDeviceBySlaveId(uint8_t slaveId);
    const std::vector<ModbusDevice>& getDevices();

} // namespace ModbusScheduler