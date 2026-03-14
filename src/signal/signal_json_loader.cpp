#include "signal/signal_json_loader.h"
#include "signal/signal_manager.h"
#include "chip/chip_manager.h"
#include "system/LogSystem.h"

/**
 * @brief Aplica solo los campos que el usuario tiene permitido editar en 
 * una señal que ya existe (especialmente si es de sistema).
 */
static void applyEditableFields(Signal& s, JsonObjectConst o) {
    if (o.containsKey("multiplier")) s.multiplier = o["multiplier"];
    if (o.containsKey("offset"))     s.offset     = o["offset"];
    if (o.containsKey("emaAlpha"))   s.emaAlpha   = o["emaAlpha"];
    // El nombre humano siempre es editable
    if (o.containsKey("name"))       s.name       = o["name"].as<std::string>();
}

/**
 * @brief Construye el objeto Signal desde un nodo JSON.
 */
static bool buildSignalFromJson(JsonObjectConst o, Signal& s) {
    const char* name = o["id"] | o["name"] | "";
    if (strlen(name) == 0) return false;

    s.name = name;
    s.chipName = o["chip"] | "";    // Nombre del Chip (ej: "SHT20_1")
    s.channelIdx = o["channel"] | 0;
    
    // Mapeo de tipo de señal
    std::string kindStr = o["kind"] | "analog_in";
    if (kindStr == "digital_in")   s.kind = SignalKind::DIGITAL_IN;
    else if (kindStr == "digital_out") s.kind = SignalKind::DIGITAL_OUT;
    else if (kindStr == "analog_out")  s.kind = SignalKind::ANALOG_OUT;
    else s.kind = SignalKind::ANALOG_IN;

    // Parámetros de procesado
    s.multiplier = o["multiplier"] | 1.0f;
    s.offset     = o["offset"]     | 0.0f;
    s.emaAlpha   = o["emaAlpha"]   | 1.0f;
    s.isReserved = o["reserved"]   | false;

    // Validación básica: El chip debe existir para que la señal sea válida
    if (s.chipName.empty()) {
        LOG_WRN("[Loader] Señal %s no tiene chip asignado", name);
    }

    return true;
}

bool loadSignalsFromJson(const JsonArrayConst& arr) {
    LOG_INF("[Loader] Cargando %d señales desde JSON...", arr.size());

    for (JsonObjectConst o : arr) {
        const char* id = o["id"] | o["name"] | "";
        if (strlen(id) == 0) continue;

        // 1. ¿Ya existe la señal en el gestor?
        Signal* existing = SignalMgr::getByName(id);

        if (existing) {
            // Si existe, solo actualizamos los campos permitidos (escalado, filtro)
            applyEditableFields(*existing, o);
            continue;
        }

        // 2. Si no existe, la creamos desde cero
        Signal s;
        if (buildSignalFromJson(o, s)) {
            SignalMgr::add(s);
        } else {
            LOG_ERR("[Loader] Error al construir señal: %s", id);
        }
    }

    return true;
}