#include "task_signal_polling.h"

#include "signal/signal_manager.h"
#include <Arduino.h>

static void taskSignalPolling(void* pvParameters)
{
    (void) pvParameters;

    const TickType_t delay = pdMS_TO_TICKS(500); // 500 ms

    for (;;)
    {
        signalManagerPollAll();
        vTaskDelay(delay);
    }
}

void startSignalPollingTask()
{
     xTaskCreatePinnedToCore(
        taskSignalPolling,
        "SignalPoll",
        4096,
        nullptr,
        5,
        nullptr,
        1
    );
}