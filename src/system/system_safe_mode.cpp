#include "system_safe_mode.h"
#include "system/nvs_store.h"
#include "system/boot_reason.h"

 bool g_safeMode = false;

bool systemInSafeMode()
{
    return g_safeMode;
}

void systemSafeModeInit()
{
    auto& nvs = NVS::sys();

    uint32_t fails = nvs.getUInt("wdtFails", 0);

    if (g_lastResetWdt)
    {
        fails++;
        nvs.putUInt("wdtFails", fails);
    }
    else
    {
        // Arranque limpio → reset contador
        fails = 0;
        nvs.putUInt("wdtFails", 0);
    }

    if (fails >= 3)
        g_safeMode = true;
}
