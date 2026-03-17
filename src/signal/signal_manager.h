#pragma once

#include <vector>
#include <string>
#include <ArduinoJson.h>
#include "signal_struct.h"

/**
 * @brief Gestor Central de Señales (Capa Lógica del PLC)
 * Este namespace coordina la transformación de datos crudos de hardware (Chips)
 * en valores de ingeniería procesados, filtrados y validados, incluyendo
 * señales remotas vía MQTT (P2P).
 */
namespace SignalMgr {

    // --- Ciclo de Vida y Memoria ---
    
    /**
     * @brief Inicializa el vector de señales y reserva memoria para evitar fragmentación.
     * @param reservedSize Cantidad de señales estimadas (por defecto 64).
     */
    void init(size_t reservedSize = 64);

    /**
     * @brief Añade una nueva señal al sistema.
     * @return true si se añadió, false si el ID ya existía o no hay memoria.
     */
    bool add(const Signal& s);

    /**
     * @brief Elimina una señal por su ID técnico.
     */
    void remove(const char* id);

    /**
     * @brief Borra todas las señales de la memoria RAM.
     */
    void clear();

    // --- Motor de Procesamiento (Runtime) ---

    /**
     * @brief Ejecuta el ciclo de actualización de todas las señales locales.
     * 1. Ejecuta el Watchdog de señales remotas (MQTT).
     * 2. Lee hardware real local (vía ChipManager).
     * 3. Procesa cálculos (Virtuales).
     * 4. Aplica filtros EMA, Escalas y Clamping.
     * @note Llamar cada 20-50ms desde el Core de Control.
     */
    void updateAll(); 

    /**
     * @brief Vigila la "frescura" de las señales que vienen por red (MQTT).
     * Si una señal BUS_MQTT no se recibe en el tiempo límite, se marca como ERROR/BAD.
     */
    void processWatchdog();

    // --- Lectura y Escritura de Datos ---

    /**
     * @brief Busca una señal por su ID único.
     */
    Signal* getById(const char* id);

    /**
     * @brief Inyecta un valor recibido desde otra placa (vía MQTT).
     * @param id ID de la señal.
     * @param val Valor crudo recibido.
     * @param quality Calidad asignada según validación de latencia/status remoto.
     */
    void updateRemote(const char* id, float val, SignalQuality quality);

    /**
     * @brief Escribe un valor en una señal (si es writable).
     * Gestiona automáticamente si la escritura es local (Chip) o remota (MQTT).
     * @return true si la operación se realizó o encoló correctamente.
     */
    bool writeSignal(const char* id, float value);

    /**
     * @brief Obtiene el valor procesado actual de una señal.
     */
    bool getValue(const char* id, float& outValue);

    // --- Acceso Masivo ---

    /**
     * @brief Devuelve la referencia al vector interno de señales.
     */
    std::vector<Signal>& getAll();

    /**
     * @brief Devuelve solo las señales que están activas y con calidad GOOD.
     */
    std::vector<Signal*> getAlivedSignals();

    /**
     * @brief Devuelve la cantidad de señales registradas.
     */
    size_t getCount();

    // --- Utilidades de Estado ---

    /**
     * @brief Cambia el modo de operación de una señal (Active, Test, Mantenimiento).
     */
    bool setMode(const char* id, SignalMode mode);

    /**
     * @brief Fuerza un valor manual si la señal está en modo TEST.
     */
    bool setManualValue(const char* id, float value);

} // namespace SignalMgr