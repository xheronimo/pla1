#include "signal/signal_persist.h"
#include "system/nvs_store.h" // Asumimos que maneja Preferences de ESP32
#include "signal/signal_manager.h"
#include "system/LogSystem.h"

/**
 * @brief Guarda la calibración de una señal específica en la NVS.
 * Se llama cada vez que el usuario pulsa "Guardar" en la interfaz.
 */
void saveSignalCalibration(const Signal& s) {
    // Solo guardamos si es analógica o si el usuario ha modificado el modo (Mantenimiento/Test)
    auto& nvs = NVS::signals();
    
    // Guardamos la estructura de calibración completa
    size_t written = nvs.putBytes(s.id, &s.calib, sizeof(SignalCalib));
    
    // También guardamos el modo de la señal (Active, Test, Mantenimiento)
    // Esto es vital para que si dejas un sensor en mantenimiento, siga así al reiniciar.
    char modeKey[36];
    snprintf(modeKey, sizeof(modeKey), "%s_m", s.id);
    nvs.putUChar(modeKey, (uint8_t)s.mode);

    if (written > 0) {
        LOG_INF("[Persist] Calibración guardada para: %s", s.id);
    }
}

/**
 * @brief Carga todas las calibraciones guardadas y las aplica a la tabla de señales en RAM.
 */
void loadSignalCalibrations() {
    auto& nvs = NVS::signals();
    auto& signals = SignalMgr::getAll();

    LOG_INF("[Persist] Cargando calibraciones desde NVS...");

    for (auto& s : signals) {
        // 1. Intentar cargar calibración (Escala, EMA, Offset)
        if (nvs.getBytes(s.id, &s.calib, sizeof(SignalCalib)) > 0) {
            // Al cargar nuevos parámetros, forzamos el reinicio de los filtros
            s.calib.emaInit = false; 
            s.status.initialized = false;
            s.quality = SignalQuality::INITIALIZING;
        }

        // 2. Intentar cargar el modo (Mantenimiento, Test, etc)
        char modeKey[36];
        snprintf(modeKey, sizeof(modeKey), "%s_m", s.id);
        if (nvs.isKey(modeKey)) {
            s.mode = (SignalMode)nvs.getUChar(modeKey, (uint8_t)SignalMode::ACTIVE);
        }
    }
}