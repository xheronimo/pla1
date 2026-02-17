#pragma once
#include <stdint.h>

#define MODBUS_MAX_BLOCKS 32
#define MODBUS_MAX_BLOCK_SIZE 16

struct ModbusBlock
{
    uint8_t slaveId;
    uint16_t startReg;
    uint8_t length;
    bool holding;
};
