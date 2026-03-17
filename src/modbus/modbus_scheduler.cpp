#include "modbus/modbus_scheduler.h"
#include "modbus/modbus_manager.h"
#include "modbus/modbus_guard.h"
#include "modbus/modbus_profiles.h"
#include "chip/chip_manager.h"
#include "system/LogSystem.h"
#include <Arduino.h>

// Variable global para el estado del bus
bool g_modbusDegraded = false;

namespace ModbusScheduler {

    // El vector dinámico que elimina el límite de MODBUS_MAX_SLAVES
    static std::vector<ModbusDevice> _devices;
    static float _busLoad = 0.0f;
    static uint32_t _totalSuccessfulReads = 0;

    void init(size_t reservedDevices) {
        _devices.clear();
        _devices.reserve(reservedDevices);
        _totalSuccessfulReads = 0;
        g_modbusDegraded = false;
        LOG_INF("[ModbusSched] Inicializado");
    }

    bool addDevice(const ModbusDevice& dev) {
        // Evitar duplicados de ID
        if (getDeviceBySlaveId(dev.id) != nullptr) return false;
        
        _devices.push_back(dev);
        LOG_INF("[ModbusSched] Dispositivo añadido: %s (ID:%u)", dev.name, dev.id);
        return true;
    }

    void clearDevices() {
        _devices.clear();
    }

    ModbusDevice* getDeviceBySlaveId(uint8_t slaveId) {
        for (auto& dev : _devices) {
            if (dev.id == slaveId) return &dev;
        }
        return nullptr;
    }

    const std::vector<ModbusDevice>& getDevices() {
        return _devices;
    }

    // --- MÉTRICAS (Sustituyen a modbus_metrics.cpp) ---

    uint32_t getTotalErrors() {
        uint32_t total = 0;
        for (const auto& dev : _devices) {
            total += dev.errorCount;
        }
        return total;
    }

    uint32_t getTotalReads() {
        return _totalSuccessfulReads;
    }

    float getBusLoad() {
        return _busLoad;
    }

    const char* getBusStatus() {
        if (_devices.empty()) return "IDLE";
        
        uint8_t timeouts = 0;
        uint8_t disabled = 0;

        for (const auto& dev : _devices) {
            if (!dev.enabled) disabled++;
            else if (dev.state == ModbusDeviceState::STATE_TIMEOUT) timeouts++;
        }

        if (disabled + timeouts == _devices.size()) return "DOWN";
        if (g_modbusDegraded || timeouts > 0) return "DEGRADED";
        return "OK";
    }

    // --- MOTOR DE POLLING ---

    void pollAll() {
        uint32_t now = millis();
        uint32_t activeBusTime = 0;
        uint32_t startTime = millis();

        for (auto& dev : _devices) {
            // 1. ¿El guardián (con Auto-Recovery) permite leer?
            if (!ModbusGuard::canRead(dev)) continue;

            // 2. Control de intervalos (Backoff si está degradado)
            uint32_t interval = dev.pollMs;
            if (g_modbusDegraded) interval *= 3; 

            if (now - dev.lastPollMs < interval) continue;

            // 3. Buscar el Chip vinculado
            Chip* chip = ChipMgr::getById(dev.name);
            if (!chip) continue;

            // 4. Ejecución física
            uint32_t tStart = millis();
            bool success = false;

            // Aquí decides qué perfil usar según el fingerprint
            if (dev.fingerprint == 0x5172)      success = ModbusProfiles::Native::pollSHT20(chip);
            else if (dev.fingerprint == 0x0630) success = ModbusProfiles::Native::pollSDM630(chip);
            // else success = ModbusProfiles::pollDynamic(chip, dev);

            activeBusTime += (millis() - tStart);
            dev.lastPollMs = millis();

            // 5. Feedback al guardián
            if (success) {
                ModbusGuard::reportSuccess(dev);
                _totalSuccessfulReads++;
            } else {
                ModbusGuard::reportError(dev);
            }

            // Yield para dejar respirar a otras tareas de FreeRTOS
            vTaskDelay(pdMS_TO_TICKS(2));
        }

        // Calcular carga del bus (0-100%)
        uint32_t totalCycleTime = millis() - startTime;
        if (totalCycleTime > 0) {
            _busLoad = (activeBusTime * 100.0f) / totalCycleTime;
        }
    }

    // --- TAREA RTOS ---

    void _taskInternal(void* pv) {
        for (;;) {
            pollAll();
            // Revisar si podemos quitar el flag de degradado tras estabilidad
            ModbusManager::checkRecovery(); 
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }

    void startTask(uint16_t stackSize, uint8_t priority) {
        xTaskCreatePinnedToCore(_taskInternal, "MB_SCHED", stackSize, nullptr, priority, nullptr, 1);
    }

} // namespace ModbusScheduler