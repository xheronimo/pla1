#include "config_apply.h"

#include "system/LogSystem.h"
#include "system/WatchdogManager.h"

//#include "net/RedManager.h"
//#include "net/mqtt_manager.h"
//#include "net/ModemManager.h"
//#include "net/TelegramHelper.h"

#include "system/RelojSistema.h"

// ==================================================
// APLICAR CONFIGURACIÓN GLOBAL
// ==================================================
void aplicarConfiguracionGlobal(const Configuracion& cfg)
{
    escribirLog("CFG:APPLY");

    // ================= RTC =================
    if (cfg.rtc.enabled)
    {
        inicializarRTC();
        cargarHoraInicial();
        iniciarTaskNtp();
        escribirLog("CFG:RTC:ON");
    }
    else
    {
        escribirLog("CFG:RTC:OFF");
    }

    // ================= RED =================
   // RedManager::init(cfg);
    escribirLog("CFG:NET");

    // ================= MODEM =================
    if (cfg.modem.enabled)
    {
       // ModemManager::init(cfg);
        escribirLog("CFG:MODEM:ON");
    }
    else
    {
        escribirLog("CFG:MODEM:OFF");
    }

    // ================= MQTT =================
    if (cfg.mqtt.enabled)
    {
        /*
        MQTTManager::configurar(
            cfg.mqtt.host,
            cfg.mqtt.port,
            cfg.mqtt.user,
            cfg.mqtt.pass,
            cfg.mqtt.baseTopic
        );
        MQTTManager::habilitar(true);
        */
        escribirLog("CFG:MQTT:ON");
    }
    else
    {/*
        MQTTManager::habilitar(false);
        escribirLog("CFG:MQTT:OFF");
        */
    }

    // ================= TELEGRAM =================
    if (cfg.telegram.enabled)
    {
        /*
        configurarTelegram(cfg.telegram.token);
        escribirLog("CFG:TG:ON");*/
    }
    else
    {
        escribirLog("CFG:TG:OFF");
    }

    // ================= WATCHDOG =================
    watchdogKick(WDT_MAIN);
}
