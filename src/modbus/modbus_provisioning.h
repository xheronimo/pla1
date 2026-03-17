#ifndef MODBUS_PROVISIONING_H
#define MODBUS_PROVISIONING_H

#include <Arduino.h>
#include <vector>

namespace ModbusProvisioning {

    struct ScanResult {
        uint8_t id;
        uint32_t baudrate;
        uint16_t fingerprint; // ID de fabricante o modelo
    };

    // --- Control de Escaneo Asíncrono ---
    void startScan(uint32_t baudrate);
    void updateScan(); // Llamar en el loop del Core 0
    bool isScanning();
    float getProgress();

    // --- Operaciones de Configuración (Bloqueantes por seguridad) ---
    bool ping(uint8_t id);
    bool changeIdSafe(uint8_t fromId, uint8_t toId, uint16_t regIdConfig);
    bool changeBaudrateSafe(uint8_t id, uint16_t regBaudConfig, uint16_t newBaudCode);

} // namespace ModbusProvisioning

#endif