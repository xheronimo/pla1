#pragma once
#include "modbus_device.h"

namespace ModbusGuard {
    // ¿Podemos leer este dispositivo ahora mismo?
    bool canRead(ModbusDevice& dev);
    
    // Notificar éxito
    void reportSuccess(ModbusDevice& dev);
    
    // Notificar error (gestiona reintentos y degradación)
    void reportError(ModbusDevice& dev);
}