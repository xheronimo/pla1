#include "system/system_fault.h"
#include "system/WatchdogManager.h"
#include "system/boot_reason.h"
#include "system/nvs_store.h"

bool g_faultAcked = false;
static uint32_t faultStartMs = 0;
static constexpr uint32_t BOOT_GRACE_MS = 8000;

bool systemHasCriticalFault()
{
    if (g_lastResetWdt)
        return true;

    if (millis() < BOOT_GRACE_MS)
        return false;

    if (watchdogIsRegistered(WDT_MAIN) &&
        watchdogLastKickAgo(WDT_MAIN) > 5000)
        return true;

    if (watchdogIsRegistered(WDT_MODBUS) &&
        watchdogLastKickAgo(WDT_MODBUS) > 5000)
        return true;

    return false;
}

void systemAcknowledgeFault()
{
    if (systemHasCriticalFault())
        return;

    g_faultAcked = true;
    NVS::sys().putBool("faultAck", true);
}

uint32_t systemFaultDurationMs()
{
    if (faultStartMs == 0)
        return 0;

    return millis() - faultStartMs;
}

void systemFaultTick()
{
    if (systemHasCriticalFault())
    {
        if (faultStartMs == 0)
            faultStartMs = millis();

        g_faultAcked = false;
    }
    else
    {
        faultStartMs = 0;
    }
}

void systemFaultInit()
{
    g_faultAcked = NVS::sys().getBool("faultAck", false);
}
