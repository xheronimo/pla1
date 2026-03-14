#include "system/fault_output.h"
#include "signal/signal_manager.h"
#include "signal/signal_write.h"
#include "system/system_fault.h"
#include "system/boot_reason.h"
#include "system/LogSystem.h"
#include "system/system_health.h"

static uint32_t lastToggle = 0;
static bool blinkState = false;
extern bool g_faultAcked;

void faultOutputInit()
{
    if (g_lastResetWdt)
        escribirLog("FAULT: Arranque tras WDT");
}

void faultOutputUpdate()
{
    Signal* s = signalManagerGetById("FAULT_WDT");
    if (!s || !s->writable)
        return;

    bool critical = systemHasCriticalFault();

    // 🟢 Sin fallo
    if (!critical)
    {
        if (g_faultAcked)
            escribirSignal(*s, 0.0f);

        blinkState = false;
        return;
    }

    // 🔴 Reset por WDT → salida fija ON
    if (g_lastResetWdt)
    {
        escribirSignal(*s, 1.0f);
        return;
    }

    // ✅ ACK válido → apagar
    if (g_faultAcked)
    {
        escribirSignal(*s, 0.0f);
        return;
    }

    // ⚠️ Fallo en caliente → parpadeo
    uint32_t now = millis();
    if (now - lastToggle > 500)
    {
        blinkState = !blinkState;
        escribirSignal(*s, blinkState ? 1.0f : 0.0f);
        lastToggle = now;
    }
}

void faultOutputClear()
{
    systemAcknowledgeFault();

    Signal* s = signalManagerGetById("FAULT_WDT");
    if (s)
        escribirSignal(*s, 0.0f);

    escribirLog("FAULT: Reconocido manualmente");
}
