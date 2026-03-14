#pragma once

#include "alarm/alarm_struct.h"
#include <stdbool.h>

/**
 * @brief Inicializa la cola de eventos de alarma.
 */
void alarm_queueInit();

/**
 * @brief Añade un evento a la cola (lo llama el AlarmMgr).
 * @return true si se pudo añadir, false si la cola está llena.
 */
bool alarm_queuePush(const AlarmEvent& event);

/**
 * @brief Extrae un evento de la cola (lo llama el Dispatcher).
 * @return true si se obtuvo un evento, false si está vacía.
 */
bool alarm_queuePop(AlarmEvent* event);