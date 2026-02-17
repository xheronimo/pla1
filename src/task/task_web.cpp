#include "task_web.h"
#include "web/web_manager.h"
#include "system/WatchdogManager.h"

#include <Arduino.h>

static void taskWeb(void* pvParameters){
    (void) pvParameters;

    watchdogRegister(WDT_NET);
    webInit();

    for (;;)
    {
        webLoop();                 // red / http / api
        watchdogKick(WDT_NET);     // 🔥 clave
        watchdogCheck();   // 👈 AQUÍ
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void startWebTask()
{
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
