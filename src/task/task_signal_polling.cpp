#include "task_signal_polling.h"
#include "signal/signal_manager.h"
#include "alarm/alarm_manager.h" // Importante: ahora están vinculados
#include "system/WatchdogManager.h"
#include <Arduino.h>

static void taskPLCScan(void* pvParameters)
{
    (void) pvParameters;

    // Reducimos a 100ms para que el PLC sea reactivo (10 veces por segundo)
    const TickType_t xFrequency = pdMS_TO_TICKS(100); 
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        // 1. Patear Watchdog (si usas el manager)
        // watchdogKick(WDT_SIGNAL_POLLING); 

        // 2. ACTUALIZAR SEÑALES (Hardware -> Lógica)
        SignalMgr::updateAll();

        // 3. EVALUAR ALARMAS (Lógica -> Interlocks de Hardware)
        // Se ejecuta inmediatamente después de las señales para mínima latencia
        AlarmMgr::update();

        // 4. Esperar de forma precisa hasta el próximo ciclo
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void startSignalPollingTask()
{
    xTaskCreatePinnedToCore(
        taskPLCScan,
        "PLC_Scan",   // Nombre más descriptivo
        8192,         // Aumentamos stack por la recursividad de las alarmas
        nullptr,
        6,            // Prioridad alta
        nullptr,
        1             // Core 1 (Core de la APP)
    );
}