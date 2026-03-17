#include "signal/signal_json_loader.h"
#include "signal/signal_manager.h"
#include "chip/chip_manager.h"
#include "system/LogSystem.h"

/**
 * @brief Aplica solo los campos editables. 
 * Útil para actualizar señales en caliente sin reiniciar el PLC.
 */
static void applyEditableFields(Signal& s, JsonObjectConst o) {
    if (o.containsKey("multiplier")) s.multiplier = o["multiplier"];
    if (o.containsKey("offset"))     s.offset     = o["offset"];
    if (o.containsKey("emaAlpha"))   s.emaAlpha   = o["emaAlpha"];
    
    if (o.containsKey("name")) {
        strlcpy(s.name, o["name"] | s.name, sizeof(s.name));
    }
}

/**
 * @brief Construye el objeto Signal desde un nodo JSON.
 */
static bool buildSignalFromJson(JsonObjectConst o, Signal& s) {
    const char* id = o["id"] | o["name"] | "";
    if (strlen(id) == 0) return false;

    // Copiamos el ID (usamos strlcpy para evitar desbordamientos en char arrays)
    strlcpy(s.id, id, sizeof(s.id));
    strlcpy(s.name, o["name"] | id, sizeof(s.name));
    strlcpy(s.chipName, o["chip"] | "", sizeof(s.chipName));
    
    s.channelIdx = o["channel"] | 0;
    
    // Mapeo de tipo de señal (Sincronizado con SignalKind Enum)
    String kindStr = o["kind"] | "analog_in";
    if (kindStr == "digital_in")        s.kind = SignalKind::DIGITAL_IN;
    else if (kindStr == "digital_out")  s.kind = SignalKind::DIGITAL_OUT;
    else if (kindStr == "analog_out")   s.kind = SignalKind::ANALOG_OUT;
    else                                s.kind = SignalKind::ANALOG_IN;

    // Parámetros de procesado y calibración
    s.multiplier = o["multiplier"] | 1.0f;
    s.offset     = o["offset"]     | 0.0f;
    s.emaAlpha   = o["emaAlpha"]   | 1.0f; // 1.0 significa sin filtro
    s.isReserved = o["reserved"]   | false;

    // Validación de hardware
    if (strlen(s.chipName) == 0 && !s.isReserved) {
        escribirLog("LOADER: WARN - Señal %s no tiene chip asignado", id);
    }

    return true;
}

/**
 * @brief Función principal llamada por el ConfigLoader
 */
bool loadSignalsFromJson(const JsonArrayConst& arr) {
    if (arr.isNull()) return false;
    
    escribirLog("LOADER: Procesando %d señales...", arr.size());

    for (JsonObjectConst o : arr) {
        const char* id = o["id"] | o["name"] | "";
        if (strlen(id) == 0) continue;

        // 1. ¿Ya existe la señal? (Para no duplicar si cargamos varias veces)
        Signal* existing = SignalMgr::getById(id);

        if (existing) {
            applyEditableFields(*existing, o);
            continue;
        }

        // 2. Si es nueva, la construimos e inyectamos
        Signal s;
        if (buildSignalFromJson(o, s)) {
            if (!SignalMgr::add(s)) {
                escribirLog("LOADER: ERR - No hay espacio para señal %s", id);
            }
        } else {
            escribirLog("LOADER: ERR - Estructura invalida para %s", id);
        }
    }

    return true;
}