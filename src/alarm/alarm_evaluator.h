#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "alarm_expr.h"

namespace AlarmMgr {

    /**
     * @brief Evalúa una expresión de alarma completa de forma recursiva.
     * Gestiona internamente la histéresis para evitar rebotes en señales analógicas.
     * * @param e Puntero al nodo raíz de la expresión.
     * @return true si la condición lógica se cumple, false en caso contrario.
     */
    bool evalExpr(const AlarmExpr* e);

    /**
     * @brief Ejecuta el ciclo de evaluación para todas las reglas registradas.
     * Detecta cambios de estado, gestiona el Latch/ACK y dispara acciones físicas.
     */
    void update();

    /**
     * @brief Función auxiliar para encontrar la señal principal involucrada en una expresión.
     * Útil para diagnósticos y saber qué señal disparó la alarma.
     */
    const char* getPrimarySignalId(const AlarmExpr* e);

} // namespace AlarmMgr