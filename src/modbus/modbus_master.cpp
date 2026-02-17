#include "modbus_master.h"
#include "modbus_guard.h"
#include "modbus_slave_context.h"
#include <ModbusMaster.h>

#include <Arduino.h>

extern ModbusMaster node;

#define MODBUS_TIMEOUT_MS 500

bool modbusReadHoldingRegisters(
    uint8_t slaveId,
    uint16_t reg,
    uint16_t count,
    uint16_t* buffer)
{
    ModbusSlaveContext* ctx = modbusGetContext(slaveId);
    if (!ctx) return false;

    

    if (!modbusGuardBeforeRead(ctx))
        return false;

    node.begin(slaveId, Serial2);

    uint8_t result = node.readHoldingRegisters(reg, count);

    if (result == node.ku8MBSuccess)
    {
        for (uint16_t i = 0; i < count; i++)
            buffer[i] = node.getResponseBuffer(i);

        modbusGuardOnSuccess(ctx);
        return true;
    }

    modbusGuardOnError(ctx);
    return false;
}
