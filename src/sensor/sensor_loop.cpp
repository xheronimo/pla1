#include "sensor_loop.h"
#include "sensor_manager.h"
#include "signal/signal_reader.h"
#include "system/WatchdogManager.h"

void taskSensors(void*)
{
    for (;;)
    {
        // aquí luego:
        // - iterar sensores
        // - leer señal
        // - evaluar sensor
        // - generar eventos

        watchdogKick(WDT_SENSORS);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
