#include "task_web.h"
#include "web/web_manager.h"

#include <Arduino.h>

void taskWeb(void* pvParameters)
{
    webInit();

    for (;;)
    {
        webLoop();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
