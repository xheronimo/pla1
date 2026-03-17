#include "task_watchdog.h"
#include "system/WatchdogManager.h"
#include "system/fault_output.h"
#include "net/mqtt_fault.h"
#include "system/system_fault.h"


void taskWatchdog(void* pv)
{
    (void)pv;
    watchdogRegister(WDT_MAIN);

    for (;;)
    {
        watchdogCheck();
        systemFaultTick();
        faultOutputUpdate();  
        mqttFaultTick();
        watchdogKick(WDT_MAIN);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
