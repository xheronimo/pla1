#pragma once
#include <stdint.h>
#include "modbus_slave_context.h"

struct ModbusBlockCache
{
    bool valid = false;
    uint8_t slave = 0;
    uint16_t startReg = 0;
    uint16_t count = 0;
    uint16_t data[MODBUS_MAX_BLOCK_REGS];
    uint32_t lastReadMs = 0;
};

