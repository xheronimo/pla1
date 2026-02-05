#include <Arduino.h>
#include "web/WebUI.h"
#include "system/WatchdogManager.h"

void taskWeb(void*)
{
    webInit();

    for (;;)
    {
        webLoop();
        watchdogKick(WDT_NET);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
