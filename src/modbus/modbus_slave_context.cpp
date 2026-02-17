#include "modbus_slave_context.h"

static ModbusSlaveContext g_ctx[MODBUS_MAX_SLAVES];

     

ModbusSlaveContext* modbusGetContext(uint8_t id)
{
    if (id == 0 || id > MODBUS_MAX_SLAVES)
        return nullptr;

    ModbusSlaveContext& ctx = g_ctx[id - 1];

    if (!ctx.inUse)
    {
        ctx.id = id;
        ctx.inUse = true;
        ctx.state = ModbusSlaveState::STATE_UNINITIALIZED;
    }

    return &ctx;
}

bool modbusRegisterAllowed(ModbusSlaveContext* ctx,
                           uint16_t start,
                           uint16_t count)
{
    if (!ctx) return false;

    uint32_t end = start + count - 1;

    if (start < ctx->minRegister) return false;
    if (end   > ctx->maxRegister) return false;

    return true;
}
