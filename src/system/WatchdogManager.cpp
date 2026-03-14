#include "system/WatchdogManager.h"
#include <esp_task_wdt.h>
#include <esp_system.h>
#include "system/LogSystem.h"

static uint32_t g_timeoutMs = 30000;

uint32_t g_lastKick[WDT_MAX];
bool g_registered[WDT_MAX];

static uint32_t g_lastCheckMs = 0;


bool watchdogIsRegistered(WatchdogId id)
{
    if (id >= WDT_MAX)
        return false;
    return g_registered[id];
}

uint32_t watchdogLastKickAgo(WatchdogId id)
{
    if (id >= WDT_MAX || !g_registered[id])
        return 0;

    return millis() - g_lastKick[id];
}

void watchdogInit(uint32_t timeoutMs)
{
    g_timeoutMs = timeoutMs;

    uint32_t now = millis();

    for (int i = 0; i < WDT_MAX; i++)
    {
        g_registered[i] = false;
        g_lastKick[i] = now;
    }

    g_lastCheckMs = now;
}

void watchdogRegister(WatchdogId id)
{
    if (id >= WDT_MAX)
        return;

    g_registered[id] = true;
    g_lastKick[id] = millis();
}

void watchdogKick(WatchdogId id)
{
    if (id >= WDT_MAX)
        return;

    if (!g_registered[id])
        return;

    g_lastKick[id] = millis();
}

// 👇 LLAMAR DESDE UNA TASK
void watchdogCheck()
{
    uint32_t now = millis();

    for (int i = 0; i < WDT_MAX; i++)
    {
        if (!g_registered[i])
            continue;

        if (now - g_lastKick[i] > g_timeoutMs)
        {
            escribirLog("WDT RESET id=%d lastKick=%lu", i, g_lastKick[i]);
            // 💥 RESET DURO (industrial)
            vTaskDelay(pdMS_TO_TICKS(50)); // deja vaciar cola SD
            esp_restart();
        }
    }

    g_lastCheckMs = now;
}
