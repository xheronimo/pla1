#include "system_init.h"

#include "system_sync.h"
#include "LogSystem.h"
#include "Display/DisplaySSD1306.h"
#include "WatchdogManager.h"

#include "RelojSistema.h"
#include "net/mqtt_manager.h"
#include "net/RedManager.h"
#include "net/ModemManager.h"
#include "config/config_struct.h"

// ==================================================
// INIT BÁSICO (SIEMPRE)
// ==================================================
void systemInitBasico()
{
    systemSyncInit();

    displayInit();

    inicializarSD();

    escribirLog("SYS:BASIC:OK");
}

// ==================================================
// INIT DEPENDIENTE DE CONFIG
// ==================================================
void systemInitConfigurado(const Configuracion& cfg)
{
    // ---------------- RED ----------------
    //RedManager::init();

    // ---------------- MODEM ----------------
    if (cfg.modem.enabled)
//        ModemManager::init();

    // ---------------- RTC ----------------
    if (cfg.rtc.enabled)
    {
        inicializarRTC();
        cargarHoraInicial();
        iniciarTaskNtp();
    }

    // ---------------- MQTT ----------------
    if (cfg.mqtt.enabled)
        //MQTTManager::habilitar(true);

    escribirLog("SYS:CFG:OK");
}
