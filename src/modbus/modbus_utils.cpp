#include "modbus_utils.h"
#include <string.h>
#include <Arduino.h>

namespace ModbusUtils {

    // --- PARSEADORES (De String a Enum) ---

    ModbusDataType parseDataType(const char* str) {
        if (!str) return ModbusDataType::UINT16;
        if (!strcasecmp(str, "int16"))  return ModbusDataType::INT16;
        if (!strcasecmp(str, "uint32")) return ModbusDataType::UINT32;
        if (!strcasecmp(str, "int32"))  return ModbusDataType::INT32;
        if (!strcasecmp(str, "float") || !strcasecmp(str, "float32")) return ModbusDataType::FLOAT32;
        return ModbusDataType::UINT16;
    }

    ModbusWordOrder parseWordOrder(const char* str) {
        if (!str) return ModbusWordOrder::AB;
        if (!strcasecmp(str, "ba"))   return ModbusWordOrder::BA;
        if (!strcasecmp(str, "cdab")) return ModbusWordOrder::CDAB;
        return ModbusWordOrder::AB;
    }

    ModbusRegType parseRegType(const char* str) {
        if (!str) return ModbusRegType::HOLDING;
        if (!strcasecmp(str, "holding"))  return ModbusRegType::HOLDING;
        if (!strcasecmp(str, "input"))    return ModbusRegType::INPUT;
        if (!strcasecmp(str, "coil"))     return ModbusRegType::COIL;
        if (!strcasecmp(str, "discrete")) return ModbusRegType::DISCRETE;
        return ModbusRegType::HOLDING;
    }

    // --- LÓGICA DE DECODIFICACIÓN ---

    uint8_t expectedWordCount(ModbusDataType type) {
        switch (type) {
            case ModbusDataType::UINT16:
            case ModbusDataType::INT16:   return 1;
            case ModbusDataType::UINT32:
            case ModbusDataType::INT32:
            case ModbusDataType::FLOAT32: return 2;
            default: return 1;
        }
    }

    float decode(const uint16_t* data, ModbusDataType type, ModbusWordOrder order) {
        if (!data) return 0.0f;

        // Caso 16 bits
        if (expectedWordCount(type) == 1) {
            return (type == ModbusDataType::INT16) ? (float)((int16_t)data[0]) : (float)data[0];
        }

        // Caso 32 bits
        union {
            uint32_t u32;
            float f32;
        } conv;

        if (order == ModbusWordOrder::AB)
            conv.u32 = ((uint32_t)data[0] << 16) | data[1];
        else if (order == ModbusWordOrder::BA)
            conv.u32 = ((uint32_t)data[1] << 16) | data[0];
        else // CDAB (Ajustar según necesidad específica de tus sensores)
            conv.u32 = ((uint32_t)data[1] << 16) | data[0];

        if (type == ModbusDataType::FLOAT32) return conv.f32;
        if (type == ModbusDataType::INT32)   return (float)((int32_t)conv.u32);
        
        return (float)conv.u32; // UINT32
    }

} // namespace ModbusUtils