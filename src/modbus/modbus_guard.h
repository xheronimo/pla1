#pragma once
#include "modbus_slave_context.h"

bool modbusGuardBeforeRead(ModbusSlaveContext* ctx);
void modbusGuardOnSuccess(ModbusSlaveContext* ctx);
void modbusGuardOnError(ModbusSlaveContext* ctx);
void modbusCheckConfigFault(ModbusSlaveContext* ctx);
