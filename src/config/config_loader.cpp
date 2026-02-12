#include "config_loader.h"

#include "signal/signal_json_loader.h"
#include "alarm/alarm_json_loader.h"
#include "system/LogSystem.h"
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "alarm/alarm_registry.h"

#include "config_defaults.h"
#include "config_global.h"
#include "config_struct.h"


bool loadConfigFromJson(
    Configuracion& cfg,
    const JsonObjectConst& root
)
{
    // ============================
    // SISTEMA
    // ============================
    JsonObjectConst sys = root["system"];
    if (!sys.isNull())
    {
        cfg.enableAlarms      = sys["alarms"]      | true;
        cfg.enablePersistence = sys["persistence"] | true;
        cfg.enableWeb         = sys["web"]          | true;
        cfg.enableMqtt        = sys["mqtt"]         | true;
        cfg.enableTelegram   = sys["telegram"]     | false;
        cfg.enableSms        = sys["sms"]           | false;

        // Modo solicitado por config (NO fuerza el arranque)
        cfg.systemMode = parseSystemMode(
            sys["mode"] | "normal"
        );
    }
    else
    {
        // Valores seguros si no existe "system"
        cfg.enableAlarms      = true;
        cfg.enablePersistence = true;
        cfg.enableWeb         = true;
        cfg.enableMqtt        = true;
        cfg.enableTelegram    = false;
        cfg.enableSms         = false;
        cfg.systemMode        = SystemMode::NORMAL;
    }

    // ============================
    // SEÑALES / SENSORES
    // ============================
    if (root.containsKey("signals"))
    {
        if (!loadSignalsFromJson(
            
                root["signals"].as<JsonArrayConst>()))
        {
            escribirLog("CFG:ERR:SIGNALS");
            return false;
        }
    }

    // ============================
    // ALARMAS
    // ============================
    if (cfg.enableAlarms && root.containsKey("alarms"))
    {
        alarmRegistryClear();

        if (!loadAlarmsFromJson(
                root["alarms"].as<JsonArrayConst>()))
        {
            escribirLog("CFG:ERR:ALARMS");
            return false;
        }
    }

    escribirLog("CFG:OK:JSON");
    return true;
}

static bool cargarDesdeArchivo(
    const char* path,
    Configuracion& cfg)
{
    File f = LittleFS.open(path, "r");
    if (!f) return false;

    JsonDocument doc;

    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) return false;

    return loadConfigFromJson(cfg, doc.as<JsonObjectConst>());

}

// ==================================================
// CARGA ROBUSTA
// ==================================================
bool cargarConfiguracion(Configuracion& cfg, SystemMode mode)
{
    if (!LittleFS.begin(true))
    {
        escribirLog("CFG:FS:FAIL");
        cargarConfigPorDefecto(cfg);
        return false;
    }

    // =========================
    // NIVEL 1 – config.json
    // =========================
    if (mode == SystemMode::NORMAL)
    {
        if (cargarDesdeArchivo("/config.json", cfg))
        {
            escribirLog("CFG:OK:config.json");
            return true;   // ← SOLO si ha ido bien
        }

        escribirLog("CFG:FAIL:config.json");
        // ⬇️ NO return → sigue al nivel 2
    }

    // =========================
    // NIVEL 2 – config_default.json
    // =========================
    if (mode != SystemMode::RECOVERY)
    {
        if (cargarDesdeArchivo("/config_default.json", cfg))
        {
            escribirLog("CFG:OK:default");
            return true;
        }

        escribirLog("CFG:FAIL:default");
        // ⬇️ NO return → sigue al nivel 3
    }

    // =========================
    // NIVEL 3 – RAM
    // =========================
    escribirLog("CFG:RAM:DEFAULT");
    cargarConfigPorDefecto(cfg);
    return false;
}

   
