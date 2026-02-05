#include "system/WatchdogManager.h"
#include <esp_task_wdt.h>

static bool registrados[WDT_MAX] = { false };

void watchdogInit(uint32_t timeoutMs)
{
    esp_task_wdt_init(timeoutMs / 1000, true);
}

void watchdogRegister(WatchdogId id)
{
    if (registrados[id]) return;

    esp_task_wdt_add(NULL);
    registrados[id] = true;
}

void watchdogKick(WatchdogId)
{
    esp_task_wdt_reset();
}
