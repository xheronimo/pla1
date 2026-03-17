#include "task_display.h"
#include  "config/config_struct.h"
#include "display/DisplaySSD1306.h"
#include <Arduino.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "system/WatchdogManager.h"

extern Configuracion cfg;
#include "task_display.h"
#include "config/config_struct.h"
#include "display/DisplaySSD1306.h"
#include "system/WatchdogManager.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern Configuracion cfg;


static void taskDisplay(void *)
{
    watchdogRegister(WDT_DISPLAY);

    for (;;)
    {
        displayLoop();
        watchdogKick(WDT_DISPLAY);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void startDisplayTask()
{
    if (!cfg.enableDisplay)
        return;

    if (!displayInit())
        return;

    xTaskCreatePinnedToCore(
        taskDisplay,
        "DISPLAY",
        4096,
        nullptr,
        1,
        nullptr,
        0
    );
}


