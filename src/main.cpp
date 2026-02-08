#include <Arduino.h>

#include "system/system_sync.h"
#include "system/WatchdogManager.h"
#include "system/LogSystem.h"

#include "config/config_loader.h"
#include "system/hardware_init.h"
#include "task/task_table.h"
#include "config/config_global.h"
#include "alarm/alarm_persistence.h"
#include "alarm/alarm_config.h"
#include "alarm/alarm_runtime.h"
#include "system/system_boot.h"
#include "config/config_defaults.h"
#include "alarm/alarm_runtime.h"
#include "task/task_load.h"


// Config global
extern Configuracion cfg;

void setup()
{
    Serial.begin(115200);
    delay(300);

    // --- Core ---
    systemSyncInit();
    watchdogInit(10000);

    // --- Hardware base (no depende de config) ---
    initHardwareBasico();

    SystemMode mode = detectSystemMode();

    switch (mode)
    {
    case SystemMode::NORMAL:
        cargarConfiguracion(cfg, mode);
        restoreAlarmRuntimeFromPersistence();
        arrancarTareasSistema(cfg);
        break;

    case SystemMode::SAFE:
        cargarConfigSafe();
        alarmRuntimeClearAll();
        arrancarTareasMinimas(); // sin alarmas
        break;

    case SystemMode::RECOVERY:
        cargarConfigRecovery();
        alarmRuntimeClearAll();
        arrancarTareasRecovery(); // web + mqtt básico
        break;
    }

    initHardwareConfigurado(cfg);

    markBootSuccess();

    escribirLog("SYS:READY");
}

void loop()
{
    watchdogKick(WDT_MAIN);
    delay(1000);
}
