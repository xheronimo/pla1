#include "task/task_modbus.h"
#include "modbus/modbus_manager.h"
#include "modbus/modbus_provisioning.h"
#include "system/WatchdogManager.h" // Importante para el Kick

/**
 * @brief Tarea de Control y Modbus (Ejecutada en Core 1)
 */
void taskModbus(void* pvParameters) {
    // 1. REGISTRO: Informamos al supervisor que esta tarea debe ser vigilada
    watchdogRegister(WDT_MODBUS);
    
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 10Hz

    escribirLog("TASK: Hilo Modbus iniciado en Core %d", xPortGetCoreID());

    for (;;) {
        // 2. KICK: "Fichamos" al inicio de cada ciclo
        watchdogKick(WDT_MODBUS);

        // Lógica de ejecución
        if (ModbusProvisioning::isScanning()) {
            ModbusProvisioning::updateScan();
        } 
        else {
            ModbusManager::pollAll();
            ModbusManager::updateWatchdog();
        }

        // 3. Esperar al siguiente ciclo (libera CPU para otras tareas de Core 1)
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}