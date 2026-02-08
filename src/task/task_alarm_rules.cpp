#include "alarm/alarm_struct.h"
#include "alarm/alarm_queue.h"
#include "system/WatchdogManager.h"

void taskAlarmRules(void*) {
    for (;;) {
        watchdogKick(WDT_ALARM_RULES);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
