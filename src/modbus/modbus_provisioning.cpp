#include "modbus/modbus_provisioning.h"
#include "modbus/modbus_manager.h"
#include "system/LogSystem.h"
#include <Arduino.h>

namespace ModbusProvisioning {

/**
 * @brief Intenta comunicación básica con un ID.
 * No usa el Guardián porque en provisioning queremos forzar la lectura 
 * aunque el dispositivo esté marcado como "error".
 */
bool ping(uint8_t id) {
    uint16_t dummy;
    // Prueba rápida: Registro 1 (Input) o Registro 0 (Holding)
    if (ModbusManager::readInputRaw(id, 0x0001, 1, &dummy)) return true;
    if (ModbusManager::readHoldingRaw(id, 0x0000, 1, &dummy)) return true;
    return false;
}

/**
 * @brief Escanea el bus en busca de dispositivos.
 * @return Número de dispositivos encontrados.
 */
size_t scan(uint32_t baudrate, std::vector<ScanResult>& results) {
    results.clear();
    
    LOG_INF("[Provisioning] Iniciando escaneo a %u bps...", baudrate);
    
    // Re-inicializamos el puerto serie para la prueba
    ModbusManager::init(baudrate); 
    vTaskDelay(pdMS_TO_TICKS(200)); 

    // Escaneo estándar Modbus (1-247)
    for (uint16_t id = 1; id <= 247; id++) {
        // Alimentar Watchdog si es necesario aquí
        
        if (ping((uint8_t)id)) {
            uint16_t fp = 0;
            // Intento de identificación por registros comunes de fabricantes
            uint16_t v;
            if (ModbusManager::readHoldingRaw(id, 0x0101, 1, &v)) fp = v; 
            
            results.push_back({(uint8_t)id, baudrate, fp});
            LOG_INF("[Provisioning] Encontrado ID: %u | FP: 0x%04X", id, fp);
        }
        
        // Pequeño respiro para no bloquear el sistema
        vTaskDelay(pdMS_TO_TICKS(5));
        yield();
    }
    
    return results.size();
}

/**
 * @brief Cambia el ID de un esclavo de forma segura.
 */
bool changeIdSafe(uint8_t fromId, uint8_t toId, uint16_t regIdConfig) {
    // 1. Verificar que el destino esté libre para evitar conflictos
    if (ping(toId)) {
        LOG_ERR("[Provisioning] Error: El ID destino %u ya está ocupado", toId);
        return false; 
    }

    // 2. Verificar que el origen responda
    if (!ping(fromId)) return false;

    // 3. Escribir nuevo ID
    LOG_INF("[Provisioning] Cambiando ID %u -> %u...", fromId, toId);
    if (!ModbusManager::writeHolding(fromId, regIdConfig, (uint16_t)toId)) return false;

    // 4. Esperar a que el sensor procese y guarde en su EEPROM/Flash
    vTaskDelay(pdMS_TO_TICKS(1000)); 
    
    return ping(toId);
}

/**
 * @brief Cambia la velocidad del esclavo.
 * Nota: Tras esto, el ESP32 perderá comunicación hasta que se reinicie el bus a la nueva velocidad.
 */
bool changeBaudrateSafe(uint8_t id, uint16_t regBaudConfig, uint16_t newBaudCode) {
    if (!ping(id)) return false;
    LOG_INF("[Provisioning] Cambiando Baudrate ID %u...", id);
    return ModbusManager::writeHolding(id, regBaudConfig, newBaudCode);
}

} // namespace ModbusProvisioning