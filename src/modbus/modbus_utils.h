#pragma once
#include <stdint.h>

// Enums globales de Modbus (pueden estar aquí o en un modbus_defs.h)
enum class ModbusDataType : uint8_t { INT16, UINT16, INT32, UINT32, FLOAT32 };
enum class ModbusWordOrder : uint8_t { AB, BA, CDAB };
enum class ModbusRegType : uint8_t  { HOLDING, INPUT, COIL, DISCRETE };

namespace ModbusUtils {

    // --- Funciones de Conversión de Texto (JSON) ---
    ModbusDataType  parseDataType(const char* str);
    ModbusWordOrder parseWordOrder(const char* str);
    ModbusRegType   parseRegType(const char* str);

    // --- Lógica de Decodificación ---
    float decode(const uint16_t* data, ModbusDataType type, ModbusWordOrder order);
    uint8_t expectedWordCount(ModbusDataType type);

} // namespace ModbusUtils