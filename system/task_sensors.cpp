#include "sensor_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "digital_manager.h"

void taskSensors(void*)
{
    sensorManagerInit();

    for (;;)
    {
        sensorManagerUpdate();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}



void taskSensors(void*)
{
    digitalManagerInit();

    for (;;)
    {
        digitalManagerLoop();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
