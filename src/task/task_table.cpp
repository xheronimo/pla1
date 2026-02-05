#include "task_table.h"

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "alarm/alarm_rules_runtime.h"

// ================================
// TASK DECLARATIONS
// ================================

extern void taskAlarmDispatcher(void*);
extern void taskAlarmRules(void*);
extern void taskDisplay(void*);
extern void taskSensors(void*);
extern void taskWeb(void*);


// (más adelante podrás añadir aquí red, mqtt, modem, etc.)

// ================================
// CREATE TASKS
// ================================

void arrancarTareasSistema(const Configuracion& cfg)
{
    // ======================
    // SENSORES
    // ======================
    xTaskCreatePinnedToCore(
        taskSensors,
        "SENSORS",
        4096,
        nullptr,
        5,
        nullptr,
        1
    );

    // ======================
    // REGLAS DE ALARMA
    // ======================
    if (cfg.state.alarms)
    {
        xTaskCreatePinnedToCore(
            taskAlarmRules,
            "ALARM_RULES",
            4096,
            nullptr,
            4,
            nullptr,
            1
        );
    }

    // ======================
    // DISPATCHER DE ALARMAS
    // ======================
    xTaskCreatePinnedToCore(
        taskAlarmDispatcher,
        "ALARM_DISPATCH",
        4096,
        nullptr,
        6,
        nullptr,
        1
    );

    // ======================
    // DISPLAY
    // ======================
    xTaskCreatePinnedToCore(
        taskDisplay,
        "DISPLAY",
        4096,
        nullptr,
        1,
        nullptr,
        0
    );

    xTaskCreatePinnedToCore(
    taskWeb,
    "WEB",
    4096,
    nullptr,
    1,
    nullptr,
    0
);

}
