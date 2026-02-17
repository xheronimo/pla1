#include <Arduino.h>
#include "system/boot_reason.h"
#include "system/LogSystem.h"
#include "system/nvs_store.h"
#include "system/system_fault.h"
#include "system/fault_output.h"
#include "system/hardware_init.h"

#include "signal/signal_manager.h"
#include "signal/signal_persist.h"
#include "signal/system_signals.h"

#include "alarm/alarm_registry.h"
#include "alarm/alarm_persistence.h"

#include "Display/DisplaySSD1306.h"
#include "web/web_manager.h"
#include "config/config_loader.h"
#include "config/config_apply.h"

#include "signal/board_fixed_signals.h"
#include "system/fault_output.h"
#include "system/system_fault.h"
#include "system/system_safe_mode.h"
#include "signal/board_fixed_signals.h"

#include "task/task_boot.h"



// --------------------------------------------------
// SAFE MODE
// --------------------------------------------------
extern bool g_safeMode;

static void checkSafeMode()
{
    auto& nvs = NVS::sys();

    uint32_t lastWdt = nvs.getUInt("lastWdtTs", 0);
    uint32_t nowTs   = millis();

    if (g_lastResetWdt)
    {
        if (lastWdt != 0 && (nowTs - lastWdt) < 600000) // 10 min
        {
            g_safeMode = true;
            escribirLog("SAFE MODE: Activado por reinicios WDT");
        }
        nvs.putUInt("lastWdtTs", nowTs);
    }
    else
    {
        nvs.putUInt("lastWdtTs", 0);
    }
}
 void setup()
{
    g_safeMode = false;
    Serial.begin(115200);

    // -------------------------
    // NVS + logs (siempre)
    // -------------------------
    inicializarSD();
    logBootReason();

    // -------------------------
    // SAFE MODE
    // -------------------------
    systemSafeModeInit();

    escribirLog("SYS: SafeMode=%s",
        systemInSafeMode() ? "ON" : "OFF");

    // -------------------------
    // Señales (registro + persistencia)
    // -------------------------
    signalManagerInit();          // registerSystem + load calib

    // -------------------------
    // Fault system
    // -------------------------
    systemFaultInit();
    faultOutputInit();

    // 4️⃣ Cargar configuración
    Configuracion cfg;
    cargarConfiguracion(cfg, SystemMode::NORMAL);

    if (systemInSafeMode())
    {
        escribirLog("SYS:Arranque en SAFE MODE");
        cfg.systemMode = SystemMode::RECOVERY;
    }

    // 5️⃣ Aplicar config global (RTC, red, etc.)
    aplicarConfiguracionGlobal(cfg);

 // 6️⃣ Señales y alarmas (solo registro + persistencia)
    signalManagerInit();
    alarmPersistenceLoadAll();

    // 7️⃣ Tareas mínimas (SIEMPRE)
    arrancarTareasMinimas();

    // 8️⃣ Tareas normales SOLO si no safe mode
    if (!systemInSafeMode())
    {
        arrancarTareasNormales();
    }
}


void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}


