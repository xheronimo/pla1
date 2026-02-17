#include "alarm/alarm_struct.h"
#include "alarm/alarm_queue.h"
#include "system/WatchdogManager.h"

#include "config/config_struct.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern Configuracion cfg;

static void taskAlarmRules(void*) {
    for (;;) {
        watchdogKick(WDT_ALARM_RULES);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void startAlarmRulesTask()
{
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
}