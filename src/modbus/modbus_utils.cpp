#include "modbus_utils.h"
#include <Arduino.h>





static float modbusToFloat(uint16_t r0, uint16_t r1, ModbusWordOrder order)
{
    union {
        uint32_t u32;
        float f;
    } v;

    if (order == ModbusWordOrder::AB)
        v.u32 = ((uint32_t)r0 << 16) | r1;
    else
        v.u32 = ((uint32_t)r1 << 16) | r0;

    return v.f;
}

ModbusDataType parseModbusDataType(const char* s)
{
    if (!s) return ModbusDataType::UINT16;
    if (!strcasecmp(s, "float")) return ModbusDataType::FLOAT32;
    if (!strcasecmp(s, "int32")) return ModbusDataType::INT32;
    return ModbusDataType::UINT16;
}

ModbusWordOrder parseWordOrder(const char* s)
{
    if (!s) return ModbusWordOrder::AB;
    if (!strcasecmp(s, "ba")) return ModbusWordOrder::BA;
    return ModbusWordOrder::AB;
}
