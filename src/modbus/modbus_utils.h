// modbus_utils.h
#pragma once
#include <stdint.h>
#include "signal/signal_struct.h"


static float modbusToFloat(uint16_t r0, uint16_t r1, ModbusWordOrder order);
ModbusDataType parseModbusDataType(const char* s);
ModbusWordOrder parseWordOrder(const char* s);


ModbusWordOrder parseModbusWordOrder(const char* str);
ModbusRegType   parseModbusRegType(const char* str);
uint8_t modbusExpectedWordCount(ModbusDataType type);
uint8_t modbusNormalizeWordCount(const Signal& s);
