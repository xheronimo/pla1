#include "modbus/modbus_provisioning.h"
#include "modbus/modbus_manager.h"
#include "system/LogSystem.h"
#include <Arduino.h>

namespace ModbusProvisioning {

    static bool _scanning = false;
    static uint8_t _currentScanId = 1;
    static uint32_t _scanBaudrate = 9600;
    static std::vector<ScanResult> _lastResults;

    /**
     * @brief Intenta comunicación básica con un ID de forma directa.
     */
    bool ping(uint8_t id) {
        uint16_t dummy[1];
        // Intentamos leer el registro 0 (Holding) o el 1 (Input)
        // Usamos la instancia interna de ModbusRTU del Manager
        ModbusRTU& mb = ModbusManager::getRawDriver(); 
        
        bool ok = false;
        // Petición síncrona corta para el ping
        mb.readHreg(id, 0, dummy, 1, [&ok](Modbus::ResultCode event, uint16_t, void*){
            if (event == Modbus::EX_SUCCESS) ok = true;
            return true;
        });

        unsigned long start = millis();
        while (mb.slave() && millis() - start < 100) { mb.task(); yield(); }
        return ok;
    }

    /**
     * @brief Inicia el proceso de escaneo sin bloquear el Core.
     */
    void startScan(uint32_t baudrate) {
        if (_scanning) return;
        _scanning = true;
        _currentScanId = 1;
        _scanBaudrate = baudrate;
        _lastResults.clear();
        escribirLog("PROV: Iniciando escaneo de bus a %u bps...", baudrate);
    }

    /**
     * @brief Procesa el escaneo ID por ID. 
     * Se llama desde el loop para no congelar la CPU.
     */
    void updateScan() {
        if (!_scanning) return;

        if (ping(_currentScanId)) {
            escribirLog("PROV: Dispositivo detectado en ID %d", _currentScanId);
            _lastResults.push_back({_currentScanId, _scanBaudrate, 0x0000});
        }

        _currentScanId++;
        
        if (_currentScanId > 247) {
            _scanning = false;
            escribirLog("PROV: Escaneo completado. Total encontrados: %d", _lastResults.size());
        }
    }

    bool isScanning() { return _scanning; }

    float getProgress() {
        return (_currentScanId / 247.0f) * 100.0f;
    }

    /**
     * @brief Cambia el ID de un esclavo verificando colisiones.
     */
    bool changeIdSafe(uint8_t fromId, uint8_t toId, uint16_t regIdConfig) {
        // 1. Verificar si el destino ya existe
        if (ping(toId)) {
            escribirLog("PROV: ERROR - El ID %d ya está ocupado en el bus.", toId);
            return false;
        }

        // 2. Escribir el nuevo ID en el registro de configuración del esclavo
        ModbusRTU& mb = ModbusManager::getRawDriver();
        bool writeOk = false;
        
        mb.writeHreg(fromId, regIdConfig, &toId, 1, [&writeOk](Modbus::ResultCode event, uint16_t, void*){
            if (event == Modbus::EX_SUCCESS) writeOk = true;
            return true;
        });

        unsigned long start = millis();
        while (mb.slave() && millis() - start < 500) { mb.task(); yield(); }

        if (writeOk) {
            escribirLog("PROV: ID cambiado %d -> %d. Verificando...", fromId, toId);
            vTaskDelay(pdMS_TO_TICKS(1000)); // Tiempo para que el esclavo reinicie su stack
            return ping(toId);
        }

        return false;
    }

    /**
     * @brief Cambia el baudrate del esclavo de forma remota.
     */
    bool changeBaudrateSafe(uint8_t id, uint16_t regBaudConfig, uint16_t newBaudCode) {
        if (!ping(id)) return false;
        
        ModbusRTU& mb = ModbusManager::getRawDriver();
        bool writeOk = false;

        mb.writeHreg(id, regBaudConfig, &newBaudCode, 1, [&writeOk](Modbus::ResultCode event, uint16_t, void*){
            if (event == Modbus::EX_SUCCESS) writeOk = true;
            return true;
        });

        unsigned long start = millis();
        while (mb.slave() && millis() - start < 500) { mb.task(); yield(); }

        if (writeOk) {
            escribirLog("PROV: Baudrate de ID %d actualizado. El bus requiere reinicio.", id);
            return true;
        }
        return false;
    }

} // namespace ModbusProvisioning