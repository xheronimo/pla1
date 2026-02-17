#include "modbus_metrics.h"
#include "modbus_slave_context.h"

uint32_t modbusGetTotalErrors()
{
    uint32_t total = 0;
    for (uint8_t i = 1; i <= MODBUS_MAX_SLAVES; i++)
    {
        ModbusSlaveContext* ctx = modbusGetContext(i);
        if (ctx && ctx->inUse)
            total += ctx->consecutiveErrors;
    }
    return total;
}

uint32_t modbusGetTotalReads()
{
    uint32_t total = 0;
    for (uint8_t i = 1; i <= MODBUS_MAX_SLAVES; i++)
    {
        ModbusSlaveContext* ctx = modbusGetContext(i);
        if (ctx && ctx->inUse)
            total += ctx->totalReads;
    }
    return total;
}