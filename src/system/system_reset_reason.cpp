#include "system/system_reset_reason.h"
#include <esp_system.h>

static const char* g_resetReason = "unknown";

void systemCaptureResetReason()
{
    esp_reset_reason_t r = esp_reset_reason();

    switch (r)
    {
        case ESP_RST_POWERON:   g_resetReason = "power_on"; break;
        case ESP_RST_PANIC:     g_resetReason = "panic"; break;
        case ESP_RST_TASK_WDT:  g_resetReason = "task_watchdog"; break;
        case ESP_RST_WDT:       g_resetReason = "watchdog"; break;
        case ESP_RST_BROWNOUT:  g_resetReason = "brownout"; break;
        case ESP_RST_SW:        g_resetReason = "software"; break;
        default:                g_resetReason = "other"; break;
    }
}

const char* systemGetResetReason()
{
    return g_resetReason;
}
