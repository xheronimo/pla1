#include "config/config_loader.h"
#include "signal/signal_json_loader.h"
#include "alarm/alarm_json_loader.h"
#include "system/LogSystem.h"
#include "storage/SDManager.h" // Usamos SDMgr para lectura robusta
#include <ArduinoJson.h>
#include "alarm/alarm_registry.h"
#include "config/config_defaults.h"
#include "config/config_global.h"
#include "config/config_struct.h"

/**
 * @brief Procesa el objeto JSON raíz para configurar el sistema, señales y alarmas.
 */
bool loadConfigFromJson(Configuracion& cfg, const JsonObjectConst& root) {
    // 1. CONFIGURACIÓN DE MÓDULOS DE SISTEMA
    JsonObjectConst sys = root["system"];
    if (!sys.isNull()) {
        cfg.enableAlarms      = sys["alarms"]      | true;
        cfg.enablePersistence = sys["persistence"] | true;
        cfg.enableWeb         = sys["web"]          | true;
        cfg.enableMqtt        = sys["mqtt"]         | true;
        cfg.enableTelegram    = sys["telegram"]     | false;
        cfg.enableSms         = sys["sms"]          | false;
        cfg.systemMode        = parseSystemMode(sys["mode"] | "normal");
    } else {
        cargarConfigPorDefecto(cfg); // Fallback si no hay nodo system
    }

    // 2. CARGA DINÁMICA DE SEÑALES (Inyecta en SignalMgr)
    if (root.containsKey("signals")) {
        // Esta función debe estar definida en signal_json_loader.cpp
        if (!loadSignalsFromJson(root["signals"].as<JsonArrayConst>())) {
            escribirLog("CFG:ERR: Fallo al procesar lista de señales");
            return false;
        }
    }

    // 3. CARGA DINÁMICA DE ALARMAS (Inyecta en AlarmRegistry)
    if (cfg.enableAlarms && root.containsKey("alarms")) {
        alarmRegistryClear();
        // Esta función debe estar definida en alarm_json_loader.cpp
        if (!loadAlarmsFromJson(root["alarms"].as<JsonArrayConst>())) {
            escribirLog("CFG:ERR: Fallo al procesar lista de alarmas");
            return false;
        }
    }

    escribirLog("CFG:OK: JSON procesado correctamente");
    return true;
}

/**
 * @brief Intenta cargar configuración desde un archivo específico en la SD.
 */
static bool cargarDesdeArchivo(const char* path, Configuracion& cfg) {
    // Usamos el SDManager que escribimos antes para mayor seguridad
    String content = SDMgr::readFile(path);
    if (content == "") return false;

    // Usamos DynamicJsonDocument para soportar archivos grandes (señales/alarmas)
    DynamicJsonDocument doc(32768); // 32KB de buffer para configuraciones densas
    DeserializationError err = deserializeJson(doc, content);

    if (err) {
        escribirLog("CFG:ERR: JSON corrupto en %s: %s", path, err.c_str());
        return false;
    }

    return loadConfigFromJson(cfg, doc.as<JsonObjectConst>());
}

// ==================================================
// CARGA ROBUSTA MULTINIVEL
// ==================================================
bool cargarConfiguracion(Configuracion& cfg, SystemMode mode) {
    
    escribirLog("CFG: Iniciando secuencia de carga (Modo: %d)", (int)mode);

    // NIVEL 1: SD - Configuración personalizada del usuario
    if (mode == SystemMode::NORMAL) {
        if (cargarDesdeArchivo("/config.json", cfg)) {
            escribirLog("CFG: Cargada configuracion de usuario (/config.json)");
            return true; 
        }
        escribirLog("CFG: No se encontro config.json en SD");
    }

    // NIVEL 2: SD - Configuración de respaldo (Default de fábrica en SD)
    if (mode != SystemMode::RECOVERY) {
        if (cargarDesdeArchivo("/config_default.json", cfg)) {
            escribirLog("CFG: Cargada configuracion por defecto desde SD");
            return true;
        }
        escribirLog("CFG: No se encontro config_default.json en SD");
    }

    // NIVEL 3: HARDCODED - El último recurso si la SD falla
    escribirLog("CFG: ADVERTENCIA - Usando valores harcoded en RAM");
    cargarConfigPorDefecto(cfg);
    return false;
}