#include "task/task_alarm_rules.h"
#include <Arduino.h>
#include "alarm/alarm_manager.h"
#include "alarm/alarm_registry.h"
#include "signal/signal_manager.h"
#include "system/WatchdogManager.h"
#include "system/LogSystem.h"
#include "chip/chip_manager.h" // Para actuar sobre relés si hay fallo

void taskAlarmRules(void* pvParameters) {
    (void)pvParameters;
    
    // Registro en el canal 1 (WDT_ALARM_RULES)
    watchdogRegister(WDT_ALARM_RULES);
    
    // Frecuencia de escaneo industrial (50ms - 100ms recomendado para seguridad)
    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    TickType_t xLastWakeTime = xTaskGetTickCount();

    escribirLog("RULES: Motor de interlocks activo en Core 1.");

    for (;;) {
        // 1. Kick al supervisor
        watchdogKick(WDT_ALARM_RULES);

        // 2. Procesar cálculos internos de señales (Virtuales/Filtros)
        signalManagerPollAll();

        // 3. Evaluar lógica de alarmas y reglas
        if (alarmRegistryCount() > 0) {
            // Esta función ejecuta el árbol de decisión
            alarmEvaluate();
        }

        // 4. ACCIÓN DE SEGURIDAD (Interlocks)
        // Si hay una alarma crítica de "Parada de Emergencia", forzamos salidas aquí
        /* if (AlarmMgr::isAnyCriticalActive()) {
            ChipMgr::forceAllRelaysOff(); 
        }
        */

        // 5. Espera determinística para mantener el ciclo constante
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void startAlarmRulesTask(int priority) {
    xTaskCreatePinnedToCore(
        taskAlarmRules,
        "AlarmRules",
        4096,
        nullptr,
        priority, // Prioridad 6 (Muy Alta)
        nullptr,
        1         // Core 1: El núcleo de ejecución lógica
    );
}