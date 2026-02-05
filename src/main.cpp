#include <Arduino.h>

#include "system/system_sync.h"
#include "system/WatchdogManager.h"
#include "system/LogSystem.h"

#include "config/config_loader.h"
#include "system/hardware_init.h"
#include "task/task_table.h"
#include "config/config_global.h"

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

    // --- Configuración ---
    cargarConfiguracion(cfg);

    // --- Hardware dependiente de config ---
    initHardwareConfigurado(cfg);

    // --- Tasks ---
    arrancarTareasSistema(cfg);
    alarmLatchLoad();

    escribirLog("SYS:READY");
}

void loop()
{
    watchdogKick(WDT_MAIN);
    delay(1000);
}
