#pragma once

#include "modbus/modbus_device.h"
#include <Arduino.h>

namespace SignalAutogen {

    /**
     * @brief Crea automáticamente las señales para un dispositivo Modbus 
     * basándose en su Fingerprint (SHT20, SDM630, etc).
     */
    void generateForDevice(const ModbusDevice& dev);

    /**
     * @brief Elimina las señales asociadas a un dispositivo (útil para limpieza).
     */
    void clearForDevice(const char* deviceName);

    /**
     * @brief Utilidad interna para construir nombres de señales (Tags).
     */
    String buildTag(uint8_t id, const char* label);

} // namespace SignalAutogen