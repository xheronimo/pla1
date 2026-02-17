#include "modbus_write.h"
#include "modbus_manager.h"
#include "modbus_guard.h"
#include "modbus_traffic.h"
#include "modbus_slave_context.h"


bool modbusWriteBlock(uint8_t slave,
                      uint16_t start,
                      uint8_t len,
                      uint16_t* values)
{
    if (!modbusTrafficAllow())
        return false;

    ModbusSlaveContext* ctx = modbusGetContext(slave);
    
    if (!modbusGuardBeforeRead(ctx))
        return false;

    bool ok = ModbusManager::writeMultipleHolding(
        slave,
        start,
        len,
        values
    );

    if (!ok)
    {
        modbusGuardOnError(ctx);
        return false;
    }

    modbusGuardOnSuccess(ctx);
    return true;
}
