#include "config_defaults.h"
#include "config_struct.h"
#include "config_global.h"
#include <Arduino.h> 

// Carga valores por defecto en RAM
void cargarConfigPorDefecto(Configuracion& cfg)
{
    
    memset(&cfg, 0, sizeof(cfg));

    // ---- STATE ----
    cfg.state.system = true;
    cfg.state.alarms = true;
    cfg.state.sms    = false;

    // ---- NETWORK ----
    strlcpy(cfg.network.apn,  "", sizeof(cfg.network.apn));
    strlcpy(cfg.network.user, "", sizeof(cfg.network.user));
    strlcpy(cfg.network.pass, "", sizeof(cfg.network.pass));
    strlcpy(cfg.network.pin,  "", sizeof(cfg.network.pin));

    // ---- MODEM ----
    cfg.modem.enabled = false;

    // ---- MQTT ----
    cfg.mqtt.enabled = false;
    cfg.mqtt.port    = 1883;

    // ---- TELEGRAM ----
    cfg.telegram.enabled = false;

    // ---- RTC ----
    cfg.rtc.enabled  = false;
    cfg.rtc.timezone = 0;

    // ---- DESTINOS ----
    for (int i = 0; i < MAX_ALARM_DESTINATIONS; i++)
    {
        cfg.alarmDestinations[i].activo = false;
        cfg.alarmDestinations[i].canales = 0;
        cfg.alarmDestinations[i].grupos  = 0;
        cfg.alarmDestinations[i].enviarRecuperacion = true;
    }

    cfg.enableAlarms      = false;
    cfg.enablePersistence = false;

    cfg.enableSms      = false;
    cfg.enableTelegram = false;
}

void cargarConfigSafe(){

    //sutb
    /*
cfg.mqtt.clientId  = "esp32-recovery";
    cfg.mqtt.baseTopic = "system/recovery";

    cfg.enableAlarms      = false;
    cfg.enablePersistence = false;

    cfg.enableSms      = false;
    cfg.enableTelegram = false;
    */

}

void cargarConfigRecovery(){

    //stub
    /*
     cfg.mqtt.clientId  = "esp32-recovery";
    cfg.mqtt.baseTopic = "system/recovery";

    cfg.enableAlarms      = false;
    cfg.enablePersistence = false;

    cfg.enableSms      = false;
    cfg.enableTelegram = false;
*/
}

