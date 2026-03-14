#pragma once

#include <vector>
#include <stdint.h>
#include "alarm/alarm_struct.h"

namespace AlarmMgr {

    /**
     * @brief Inicializa el motor de alarmas y la cola de eventos interna.
     * Debe llamarse en el setup() antes de arrancar las tareas.
     */
    void init();

    /**
     * @brief Añade una nueva regla de alarma al vector dinámico.
     * @param rule Estructura con la configuración de la alarma.
     */
    void addRule(const AlarmRule& rule);

    /**
     * @brief Limpia todas las reglas y libera la memoria de los árboles de expresiones.
     */
    void clearRules();

    /**
     * @brief Ciclo principal de evaluación. 
     * Recorre todas las alarmas, evalúa condiciones con histéresis y ejecuta 
     * interlocks de hardware inmediatos.
     */
    void update();

    /**
     * @brief Reconoce (ACK) una alarma activa. 
     * Si la alarma es de tipo latch, esto permitirá que se recupere.
     * @param alarmId ID único de la alarma.
     */
    void acknowledge(uint32_t alarmId);

    // --- Getters y Utilidades ---
    
    /**
     * @brief Busca una regla por su ID.
     * @return Puntero a la regla o nullptr si no existe.
     */
    AlarmRule* getById(uint32_t id);

    /**
     * @brief Retorna la referencia al vector de reglas (para la UI o Web).
     */
    std::vector<AlarmRule>& getRules();

    /**
     * @brief Cuenta cuántas alarmas están actualmente en estado activo.
     */
    uint32_t getActiveCount();

} // namespace AlarmMgr