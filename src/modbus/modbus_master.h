#pragma once
#include <stdint.h>
#include "modbus_slave_context.h"

bool modbusReadHoldingRegisters(
    uint8_t slaveId,
    uint16_t reg,
    uint16_t count,
    uint16_t* buffer);
