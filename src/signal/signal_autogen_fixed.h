#pragma once

/**
 * @brief Genera las señales fijas de la PCB (PCF8574, ADC, GPIO Multipropósito).
 * Se debe llamar una sola vez durante el setup(), después de inicializar los buses.
 */
void signalAutogenFixedGenerateAll();