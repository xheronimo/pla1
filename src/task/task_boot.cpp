#include "task/task_boot.h"
#include "system/LogSystem.h"
#include "system/system_safe_mode.h"

// Tareas de Sistema
#include "task_watchdog.h"
#include "task_web.h"
#include "task_display.h"
#include "task_i2c_recovery.h"

// Tareas de Lógica de PLC (Las que hemos actualizado)
#include "task_signal_polling.h"
#include "task_alarm_rules.h"
#include "task_alarm_dispatcher.h"
#include "modbus/modbus_poll_task.h"

/**
 * @brief Tareas mínimas que arrancan SIEMPRE (Incluso en Safe Mode)
 * Priorizamos la conectividad para poder arreglar el PLC vía Web.
 */
void arrancarTareasMinimas() {
    escribirLog("BOOT: Arrancando tareas mínimas de sistema...");

    // Watchdog: Prioridad máxima para supervisar todo
    startWatchdogTask(1); 

    // Web y Display: Prioridad media/baja
    startWebTask(3);
    startDisplayTask(4);
    
    // Recuperación I2C: Prioridad alta (si el bus cae, nada funciona)
    startI2CRecoveryTask(2);
}

/**
 * @brief Tareas de operación NORMAL
 * Aquí es donde vive la "inteligencia" del PLC.
 */
void arrancarTareasNormales() {
    if (systemInSafeMode()) {
        escribirLog("BOOT: Ignorando tareas normales por SAFE MODE");
        return;
    }

    escribirLog("BOOT: Lanzando motor de ejecución del PLC...");

    // 1. POLLING DE SEÑALES (Prioridad 5 - Muy Alta)
    // Es la base de datos del sistema. Debe ser constante.
    startSignalPollingTask(5);

    // 2. REGLAS DE ALARMA (Prioridad 6 - CRÍTICA)
    // Es la tarea que ejecuta los Interlocks. Debe ir rápido.
    startAlarmRulesTask(6);

    // 3. DISPATCHER DE EVENTOS (Prioridad 3 - Media)
    // Envía Telegram/MQTT. Si la red va lenta, no debe frenar al PLC.
    startAlarmDispatcherTask(3);

    // 4. MODBUS (Prioridad 4 - Media/Alta)
    // Depende de los tiempos de respuesta de los esclavos.
    startModbusTask(4);
}