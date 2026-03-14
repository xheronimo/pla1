#pragma once

#include <vector>
#include <string>
#include <ArduinoJson.h>
#include "signal_struct.h"

/**
 * @brief Gestor Central de Señales (Capa Lógica del PLC)
 * * Este namespace coordina la transformación de datos crudos de hardware (Chips)
 * en valores de ingeniería procesados, filtrados y validados.
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
     * @brief Ejecuta el ciclo de actualización de todas las señales.
     * 1. Lee hardware real (vía ChipManager).
     * 2. Procesa cálculos (Virtuales).
     * 3. Aplica filtros EMA, Escalas y Clamping.
     * @note Se recomienda llamar a esta función cada 20-50ms.
     */
    void pollAll();

    // --- Lectura y Escritura de Datos ---

    /**
     * @brief Busca una señal por su ID único.
     */
    Signal* getById(const char* id);

    /**
     * @brief Escribe un valor en una señal (si es writable).
     * Gestiona automáticamente la escritura en el Chip correspondiente.
     * @return true si la operación se encoló correctamente en el Chip.
     */
    bool writeSignal(const char* id, float value);

    /**
     * @brief Obtiene el valor procesado actual de una señal.
     */
    bool getValue(const char* id, float& outValue);

    // --- Acceso Masivo ---

    /**
     * @brief Devuelve la referencia al vector interno de señales.
     * Útil para serialización masiva o dashboards.
     */
    std::vector<Signal>& getAll();

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