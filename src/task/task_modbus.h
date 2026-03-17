#ifndef TASK_MODBUS_H
#define TASK_MODBUS_H

#include <Arduino.h>

/**
 * @brief Tarea principal para la gestión del bus Modbus RTU.
 * Se encarga de alternar entre el modo de producción (lectura de señales)
 * y el modo de aprovisionamiento (escaneo de nuevos dispositivos).
 * * @param pvParameters Parámetros de FreeRTOS (no utilizados).
 */
void taskModbus(void* pvParameters);

#endif