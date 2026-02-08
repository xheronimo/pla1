#include "task_signal_polling.h"

#include "signal/signal_manager.h"
#include <Arduino.h>

void taskSignalPolling(void* pvParameters)
{
    (void) pvParameters;

    const TickType_t delay = pdMS_TO_TICKS(500); // 500 ms

    for (;;)
    {
        signalManagerPollAll();
        vTaskDelay(delay);
    }
}
