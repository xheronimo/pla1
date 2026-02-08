#include "task/task_alarm_evaluator.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "alarm/alarm_evaluator.h"

#include "alarm/alarm_config.h"
#include "signal/signal_manager.h"
#include "alarm/alarm_registry.h"

void taskAlarmEvaluator(void *arg)
{
    const AlarmRule *rules = nullptr;
    size_t ruleCount = 0;

    // Obtener reglas (config / memoria)
    alarmGetRules(rules, ruleCount);

    while (true)
    {

        // Actualizar señales
        signalManagerPollAll();

        // Evaluar alarmas
        // 2. Solo evaluar alarmas si hay reglas
        if (alarmRegistryCount() > 0)
        {
            alarmEvaluate();
        }
        vTaskDelay(pdMS_TO_TICKS(200)); // periodo de evaluación
    }
}