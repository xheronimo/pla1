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



ModbusWordOrder parseWordOrder(const char* s)
{
    if (!s) return ModbusWordOrder::AB;
    if (!strcasecmp(s, "ba")) return ModbusWordOrder::BA;
    return ModbusWordOrder::AB;
}

// -------------------------------------
// MODBUS DATA TYPE
// -------------------------------------

ModbusDataType parseModbusDataType(const char* str)
{
    if (!str) return ModbusDataType::UINT16;

    if (strcmp(str, "int16") == 0)  return ModbusDataType::INT16;
    if (strcmp(str, "uint32") == 0) return ModbusDataType::UINT32;
    if (strcmp(str, "int32") == 0)  return ModbusDataType::INT32;
    if (strcmp(str, "float") == 0)  return ModbusDataType::FLOAT32;

    return ModbusDataType::UINT16;
}

// -------------------------------------
// MODBUS WORD ORDER
// -------------------------------------

ModbusWordOrder parseModbusWordOrder(const char* str)
{
    if (!str) return ModbusWordOrder::AB;

    if (strcmp(str, "ab") == 0) return ModbusWordOrder::AB;
    if (strcmp(str, "ba") == 0) return ModbusWordOrder::BA;
    if (strcmp(str, "cdab") == 0) return ModbusWordOrder::CDAB;

    return ModbusWordOrder::AB;
}

// -------------------------------------
// MODBUS REGISTER TYPE
// -------------------------------------

ModbusRegType parseModbusRegType(const char* str)
{
    if (!str) return ModbusRegType::MODBUS_HOLDING;

    if (strcmp(str, "holding") == 0)  return ModbusRegType::MODBUS_HOLDING;
    if (strcmp(str, "input") == 0)    return ModbusRegType::MODBUS_INPUT;
    if (strcmp(str, "coil") == 0)     return ModbusRegType:: MODBUS_COIL;
    if (strcmp(str, "discrete") == 0) return ModbusRegType::MODBUS_DISCRETE_INPUT;

    return ModbusRegType::MODBUS_HOLDING;
}

uint8_t modbusExpectedWordCount(ModbusDataType type)
{
    switch (type)
    {
        case ModbusDataType::UINT16:
        case ModbusDataType::INT16:
            return 1;

        case ModbusDataType::UINT32:
        case ModbusDataType::INT32:
        case ModbusDataType::FLOAT32:
            return 2;
    }
    return 1;
}

uint8_t modbusNormalizeWordCount(const Signal& s)
{
    uint8_t expected = modbusExpectedWordCount(s.dataType);

    if (s.wordCount == 0 || s.wordCount != expected)
        return expected;

    return s.wordCount;
}


