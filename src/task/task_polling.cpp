#include "tasks/task_polling.h"
#include "system/WatchdogManager.h"
#include "net/NetworkManager.h"
#include "system/LogSystem.h"

void taskPolling(void* pvParameters) {
    // 1. Registro de los módulos que esta tarea gestiona directamente
    watchdogRegister(WDT_NET);
    watchdogRegister(WDT_MAIN); // Opcional: registrar el flujo principal aquí

    escribirLog("TASK: Hilo de Polling y Supervisión iniciado en Core %d", xPortGetCoreID());

    for (;;) {
        // --- A. SUPERVISIÓN CRÍTICA ---
        // Verifica si todos los IDs (MODBUS, PLC_SCAN, MQTT, etc.) han fichado.
        // Si alguno falló, watchdogCheck() ejecutará el esp_restart().
        watchdogCheck();

        // --- B. SERVICIOS DE RED ---
        // Mueve el stack de WebSockets para el Dashboard
        NetworkMgr::broadcastSignals();
        
        // Verifica la seguridad del AP y clientes conectados
        NetworkMgr::manageAPSecurity();

        // --- C. FEEDBACK DEL SISTEMA ---
        // Si tienes una pantalla o LED de estado, este es el lugar para actualizarlo
        // watchdogKick(WDT_DISPLAY); 

        // --- D. KICK DE SUPERVIVENCIA ---
        // Indicamos que el hilo de polling/red sigue vivo
        watchdogKick(WDT_NET);

        // Frecuencia de ejecución: 1 segundo (estándar para supervisión)
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}