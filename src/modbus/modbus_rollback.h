#pragma once
#include <Arduino.h>
#include "modbus_state.h"

/**
 * @brief Módulo de Seguridad y Autorreparación del Bus Modbus.
 * * Este módulo se encarga de monitorear la salud del bus y realizar un 
 * "Rollback" (vuelta atrás) a una configuración estable si detecta que 
 * el bus ha quedado inoperativo tras una provisión o cambio de parámetros.
 */
namespace ModbusRollback {

    // --- GESTIÓN DE SNAPSHOTS (NVS) ---
    
    /**
     * @brief Guarda la configuración actual (ID, FP, Enabled) como estado estable.
     * Se debe llamar automáticamente tras 30-60 segundos de operación sin errores.
     */
    void saveStableSnapshot();

    /**
     * @brief Restaura los dispositivos desde la última configuración estable guardada.
     * @return true si se pudo restaurar, false si no había snapshot válido.
     */
    bool performRollback();

    // --- MONITOREO EN TIEMPO REAL ---

    /**
     * @brief Analiza la salud del bus y decide si es necesario un rollback.
     * Debe ejecutarse en cada ciclo del taskModbusPoll.
     */
    void monitorStability();

    /**
     * @brief Resetea los contadores de estabilidad. 
     * Se usa tras un cambio manual para iniciar una nueva ventana de validación.
     */
    void resetStabilityTimer();

    // --- UTILIDADES ---

    /**
     * @brief Verifica si el bus ha estado libre de errores críticos durante X tiempo.
     * @param thresholdMs Tiempo requerido en milisegundos.
     */
    bool isBusStable(uint32_t thresholdMs);
}