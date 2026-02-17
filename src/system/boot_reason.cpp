#include "system/boot_reason.h"
#include "system/LogSystem.h"
#include "system/nvs_store.h"
#include "esp_system.h"

uint32_t g_wdtResetCount = 0;
bool g_lastResetPower = false;
bool g_lastResetSoftware = false;
bool g_lastResetWdt = false;

void logBootReason()
{
    g_lastResetPower    = false;
    g_lastResetSoftware = false;
    g_lastResetWdt      = false;

    auto& nvs = NVS::sys();
    g_wdtResetCount = nvs.getUInt("wdtCount", 0);

    esp_reset_reason_t r = esp_reset_reason();

    switch (r)
    {
    case ESP_RST_POWERON:
        g_lastResetPower = true;
        escribirLog("BOOT: Power ON");
        break;

    case ESP_RST_SW:
        g_lastResetSoftware = true;
        escribirLog("BOOT: Software reset");
        break;

    case ESP_RST_WDT:
    case ESP_RST_TASK_WDT:
        g_lastResetWdt = true;
        g_wdtResetCount++;
        nvs.putUInt("wdtCount", g_wdtResetCount);
        escribirLog("BOOT: Watchdog reset (%lu)", g_wdtResetCount);
        break;

    case ESP_RST_BROWNOUT:
        escribirLog("BOOT: Brownout");
        break;

    default:
        escribirLog("BOOT: Reset reason %d", (int)r);
        break;
    }

    if (g_lastResetWdt)
        escribirLog("WARN: Último reset por watchdog");
}

