#pragma once
#include <stdint.h>

struct ModbusBlockCache
{
    bool valid = false;
    uint8_t slaveId;
    uint16_t startReg;
    uint8_t length;
    uint16_t values[MODBUS_MAX_BLOCK_SIZE];
    uint32_t lastReadMs = 0;
};
