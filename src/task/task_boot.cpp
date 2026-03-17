#include "task/task_boot.h"
#include <Arduino.h>

// Managers y Sistemas
#include "system/LogSystem.h"
#include "system/system_safe_mode.h"
#include "system/WatchdogManager.h"
#include "storage/SDManager.h"
#include "chip/chip_manager.h"
#include "signal/signal_manager.h"
#include "alarm/alarm_manager.h"
#include "modbus/modbus_manager.h"
#include "net/NetworkManager.h"

// Tareas
#include "task/task_polling.h"           // Core 0: WDT + Web
#include "task/task_i2c_recovery.h"     // Core 0: Mantenimiento I2C
#include "task/task_alarm_dispatcher.h" // Core 0: Red/Notificaciones
#include "task/task_modbus.h"           // Core 1: Bus RS485
#include "task/task_alarm_rules.h"      // Core 1: Lógica/Interlocks

/**
 * @brief CAPA 1: Tareas de Supervivencia
 * Estas arrancan siempre, incluso si el PLC está en modo rescate.
 */
void arrancarTareasMinimas() {
    escribirLog("BOOT: Iniciando Capa 1 (Supervivencia)");

    // 1. El Supervisor Global (Core 0)
    // Gestiona watchdogCheck() y el broadcast de WebSockets
    xTaskCreatePinnedToCore(taskPolling, "TaskPolling", 8192, NULL, 1, NULL, 0);

    // 2. Recuperador de hardware (Core 0)
    // Vigila que los PCF8574 no bloqueen el bus I2C
    startI2CRecoveryTask();

    // 3. Servidor Web (Core 0)
    // NetworkMgr::init ya fue llamado en el main para registrar las rutas API
    escribirLog("BOOT: Capa 1 OK.");
}

/**
 * @brief CAPA 2: Operación Normal del PLC
 * Solo arranca si el hardware y la configuración son estables.
 */
void arrancarTareasNormales() {
    if (systemInSafeMode()) {
        escribirLog("BOOT: Modo SEGURO activo. Omitiendo Capa 2.");
        return;
    }

    escribirLog("BOOT: Iniciando Capa 2 (Control Industrial)");

    // 1. Inicializar Bus de Campo (Modbus)
    // Primero optimizamos los grupos basados en las señales cargadas
    ModbusManager::autoBuildGroups();
    
    // 2. Lanzar Tarea de Lectura Modbus (Core 1)
    // Prioridad 4: Debe ser constante para el muestreo
    startModbusTask(4);

    // 3. Lanzar Motor de Reglas e Interlocks (Core 1)
    // Prioridad 6: La más alta. La seguridad no puede esperar.
    startAlarmRulesTask(6);

    // 4. Lanzar Despachador de Alarmas (Core 0)
    // Prioridad 3: Gestión de MQTT/Telegram sin frenar el control
    startAlarmDispatcherTask();

    escribirLog("BOOT: PLC en ejecución normal (Multicore).");
}