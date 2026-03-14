#include "net/mqtt_manager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void taskMQTT(void*)
{
    MQTTManager::init();

    for (;;)
    {
        MQTTManager::loop();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
