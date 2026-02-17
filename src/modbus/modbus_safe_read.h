#pragma once
#include <ModbusMaster.h>
#include "modbus_slave_context.h"

#define MODBUS_RESPONSE_TIMEOUT_MS 500

bool modbusSafeReadHolding(
    ModbusMaster& node,
    ModbusSlaveContext* ctx,
    uint16_t addr,
    uint16_t qty,
    uint16_t* buffer
);
