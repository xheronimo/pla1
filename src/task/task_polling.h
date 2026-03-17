#ifndef TASK_POLLING_H
#define TASK_POLLING_H

/**
 * @brief Tarea de supervisión y gestión de red.
 * Ejecuta el Watchdog Check y servicios de mantenimiento.
 * Normalmente se asigna al Core 0.
 */
void taskPolling(void* pvParameters);

#endif