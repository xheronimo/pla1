#include "system/hardware_init.h"

#include <Arduino.h>

#include "system/LogSystem.h"

// HW básico
#include "system/BusI2C.h"
#include "Display/DisplaySSD1306.h"


// Opcionales (pueden ser stubs)
#include "RelojSistema.h"

// ==================================================
// A) HARDWARE BÁSICO (NO depende de config)
// ==================================================
void initHardwareBasico()
{
    // ---------------- I2C ----------------
    if (BusI2C_Init())
        escribirLog("HW:I2C:OK");
    else
        escribirLog("HW:I2C:FAIL");

    // ---------------- DISPLAY ----------------
    if (displayInit())
        escribirLog("HW:DISPLAY:OK");
    else
        escribirLog("HW:DISPLAY:FAIL");

    // ---------------- SD ----------------
    if (inicializarSD())
        escribirLog("HW:SD:OK");
    else
        escribirLog("HW:SD:FAIL");

    // ---------------- RS485 / MODBUS (solo HW) ----------------
    #ifdef USE_MODBUS
        ModbusManager::init();
        escribirLog("HW:RS485:OK");
    #endif
}

// ==================================================
// B) HARDWARE DEPENDIENTE DE CONFIG
// ==================================================
void initHardwareConfigurado(const Configuracion& cfg)
{
    // ---------------- RED ----------------
    #ifdef USE_NETWORK
        RedManager::init(cfg);
        escribirLog("HW:NET:INIT");
    #endif

    // ---------------- MODEM ----------------
    #ifdef USE_MODEM
        if (cfg.modem.enabled)
        {
            ModemManager::init(cfg);
            escribirLog("HW:MODEM:ON");
        }
    #endif

    // ---------------- MQTT ----------------
    #ifdef USE_MQTT
        if (cfg.mqtt.enabled)
        {
            MQTTManager::configurar(
                cfg.mqtt.host,
                cfg.mqtt.port,
                cfg.mqtt.user,
                cfg.mqtt.pass,
                cfg.mqtt.baseTopic
            );
            MQTTManager::habilitar(true);
            escribirLog("HW:MQTT:ON");
        }
    #endif

    // ---------------- RTC / NTP ----------------
    if (cfg.rtc.enabled)
    {
        inicializarRTC();
        cargarHoraInicial();
        iniciarTaskNtp();
        escribirLog("HW:RTC:NTP");
    }
}
