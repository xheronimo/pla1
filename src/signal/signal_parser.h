#pragma once

#include "signal_struct.h"
#include "board/board_config.h" // Para I2CDevice si está allí definido

/**
 * @brief Utilidades para convertir texto (JSON/Web) a tipos enumerados de señales.
 */

/**
 * @brief Convierte string a SignalKind (Tipo de señal: analog_in, digital_in, etc.)
 */
SignalKind parseSignalKind(const char* str);

/**
 * @brief Convierte string a BusType (Origen: gpio, i2c, modbus, pcf, onewire)
 */
BusType parseBusType(const char* str);

/**
 * @brief Convierte string a SignalMode (active, test, offline)
 */
SignalMode parseSignalMode(const char* str);

/**
 * @brief Identifica el modelo de Chip I2C desde un string
 */
I2CDevice parseI2CDevice(const char* str);

/**
 * @brief (Opcional) Utilidades inversas para devolver strings (útil para la API JSON)
 */
const char* signalKindToString(SignalKind kind);
const char* busTypeToString(BusType bus);
const char* signalModeToString(SignalMode mode);