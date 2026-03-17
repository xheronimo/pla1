#include "config/config_defaults.h"
#include "config/config_struct.h"
#include "config/config_global.h"
#include "system/LogSystem.h"
#include <Arduino.h> 

/**
 * @brief Carga valores de emergencia absoluta en RAM.
 * Se usa cuando no hay SD o el JSON es totalmente ilegible.
 */
void cargarConfigPorDefecto(Configuracion& cfg)
{
    // Limpieza inicial de la estructura
    memset(&cfg, 0, sizeof(cfg));

    // ---- ESTADO INICIAL ----
    cfg.enableWeb         = true;  // Siempre web para poder configurar
    cfg.enableAlarms      = false; // Desactivadas por seguridad en default
    cfg.enablePersistence = false; 
    cfg.systemMode        = SystemMode::NORMAL;

    // ---- NETWORK (AP por defecto si no hay nada) ----
    strlcpy(cfg.network.apn,  "", sizeof(cfg.network.apn));
    strlcpy(cfg.network.user, "", sizeof(cfg.network.user));
    
    // ---- MQTT (Desactivado por defecto) ----
    cfg.mqtt.enabled = false;
    cfg.mqtt.port    = 1883;
    strlcpy(cfg.mqtt.clientId, "PLC-ESP32-GENERIC", sizeof(cfg.mqtt.clientId));

    // ---- MODEM & TELEGRAM ----
    cfg.modem.enabled    = false;
    cfg.telegram.enabled = false;

    // ---- RTC ----
    cfg.rtc.enabled  = false;
    cfg.rtc.timezone = 0;

    // ---- DESTINOS DE ALARMA ----
    for (int i = 0; i < MAX_ALARM_DESTINATIONS; i++)
    {
        cfg.alarmDestinations[i].activo = false;
        cfg.alarmDestinations[i].canales = 0; // Bitmask vacío
        cfg.alarmDestinations[i].grupos  = 0;
        cfg.alarmDestinations[i].enviarRecuperacion = true;
    }

    escribirLog("DEFAULTS: Configuracion de emergencia cargada en RAM.");
}

/**
 * @brief Configuracion para SAFE MODE.
 * El sistema arranca pero minimiza procesos para evitar colisiones de hardware.
 */
void cargarConfigSafe(Configuracion& cfg) {
    // Primero cargamos base default
    cargarConfigPorDefecto(cfg);
    
    cfg.systemMode = SystemMode::SAFE;
    
    // Forzamos desactivacion de buses de campo
    cfg.enableAlarms = false;
    cfg.mqtt.enabled = false;
    
    // Identificador especial para debug
    strlcpy(cfg.mqtt.clientId, "PLC-SAFE-MODE", sizeof(cfg.mqtt.clientId));
    
    escribirLog("DEFAULTS: Aplicado perfil SAFE MODE.");
}

/**
 * @brief Configuracion para RECOVERY MODE.
 * Prioridad absoluta a la red y al servidor web para reflash/configuracion.
 */
void cargarConfigRecovery(Configuracion& cfg) {
    cargarConfigPorDefecto(cfg);
    
    cfg.systemMode = SystemMode::RECOVERY;
    
    // En recuperación solemos usar un tópico MQTT distinto para no ensuciar datos
    strlcpy(cfg.mqtt.clientId, "PLC-RECOVERY", sizeof(cfg.mqtt.clientId));
    strlcpy(cfg.mqtt.baseTopic, "system/recovery", sizeof(cfg.mqtt.baseTopic));

    cfg.enableWeb = true;
    cfg.enableAlarms = false;
    cfg.enablePersistence = false;
    
    escribirLog("DEFAULTS: Aplicado perfil RECOVERY MODE.");
}