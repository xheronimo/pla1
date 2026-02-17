#include "modbus_safe_read.h"
#include "modbus_guard.h"



bool modbusSafeReadHolding(
    ModbusMaster& node,
    ModbusSlaveContext* ctx,
    uint16_t addr,
    uint16_t qty,
    uint16_t* buffer
)
{
    if (!ctx) return false;

    uint32_t now = millis();

    if (!modbusGuardBeforeRead(ctx))
        return false;

    node.clearResponseBuffer();

    uint8_t result = node.readHoldingRegisters(addr, qty);

    if (result != node.ku8MBSuccess)
    {
        modbusGuardOnError(ctx);
        return false;
    }

    for (uint16_t i = 0; i < qty; i++)
        buffer[i] = node.getResponseBuffer(i);

    modbusGuardOnSuccess(ctx);
    return true;
}
