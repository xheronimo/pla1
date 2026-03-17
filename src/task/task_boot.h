#pragma once

/**
 * @brief Arranca las tareas esenciales de supervisión y conectividad.
 * Se ejecuta SIEMPRE, incluso si el sistema está en SAFE MODE.
 * Incluye: Watchdog, Web Server, I2C Recovery y Display.
 */
void arrancarTareasMinimas();

/**
 * @brief Arranca el motor de ejecución del PLC (Polling, Reglas, Modbus).
 * Solo se ejecuta si el sistema NO está en SAFE MODE.
 * Si el sistema detecta inestabilidad, estas tareas se omiten para 
 * evitar movimientos no deseados de los actuadores.
 */
void arrancarTareasNormales();