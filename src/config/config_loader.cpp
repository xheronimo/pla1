#include "config_loader.h"
#include "config/config_defaults.h"

#include "system/LogSystem.h"
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "config/config_struct.h"


bool loadConfigFromJson(
    Configuracion& cfg,
    const ArduinoJson::JsonDocument& doc
)
{
    // 1️⃣ Cargar defaults primero (base segura)
    cargarConfigPorDefecto(cfg);

    // 2️⃣ Si el JSON está vacío, salimos
    if (doc.isNull())
    {
        escribirLog("CFG:JSON:EMPTY");
        return false;
    }

    // 3️⃣ Mínimo viable: marcar que la config es válida
    // (aquí luego irás metiendo secciones)
    escribirLog("CFG:JSON:LOADED");

    return true;
}


static bool cargarDesdeArchivo(
    const char* path,
    Configuracion& cfg)
{
    File f = LittleFS.open(path, "r");
    if (!f) return false;

    StaticJsonDocument<4096> doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) return false;

    return loadConfigFromJson(cfg, doc);
}

// ==================================================
// CARGA ROBUSTA
// ==================================================
bool cargarConfiguracion(Configuracion& cfg)
{
    if (!LittleFS.begin(true))
    {
        escribirLog("CFG:FS:FAIL");
        cargarConfigPorDefecto(cfg);
        return false;
    }

    // 1️⃣ config.json
    if (cargarDesdeArchivo("/config.json", cfg))
    {
        escribirLog("CFG:OK:config.json");
        return true;
    }

    escribirLog("CFG:FAIL:config.json");

    // 2️⃣ config_default.json
    if (cargarDesdeArchivo("/config_default.json", cfg))
    {/*  */
        escribirLog("CFG:OK:default");
        return true;
    }

    escribirLog("CFG:FAIL:default");

    // 3️⃣ RAM defaults
    cargarConfigPorDefecto(cfg);
    escribirLog("CFG:RAM:DEFAULT");

    return false;
}
