#include "modbus_poll.h"
#include "modbus_manager.h"
#include "signal/signal_registry.h"
#include "modbus_slave_context.h"
#include "modbus_guard.h"
#include "modbus_decode.h"
#include "modbus_utils.h"
#include <Arduino.h>

void pollSlaveByType(uint8_t slave, ModbusRegType type)
{
    size_t count;
    Signal *table = signalRegistryGetAll(count);

    if (!table)
        return;

    ModbusSlaveContext *ctx = modbusGetContext(slave);
    if (!modbusGuardBeforeRead(ctx))
        return;
    ctx->lastPollTs = millis(); // 👈 aquí
    bool slaveOk = false;
    bool slaveError = false;

    for (size_t i = 0; i < count;)
    {
        Signal &s = table[i];

        if (s.bus != BusType::BUS_MODBUS ||
            s.address != slave ||
            s.modbusType != type ||
            s.writable)
        {
            i++;
            continue;
        }

        uint16_t start = s.channel;
        uint16_t wc = modbusNormalizeWordCount(s);
        uint16_t buffer[MODBUS_MAX_BLOCK_REGS];

        if (ModbusManager::readBlock(slave, type, start, wc, buffer))
        {
            float v;
            if (modbusDecodeValue(buffer, s, v))
            {
                s.raw = v;
                s.value = v;
                s.valid = true;
                slaveOk = true;
            }
            else
            {
                slaveError = true;
                ctx->configErrors++;
                modbusCheckConfigFault(ctx);
            }
        }
        else
        {
            slaveError = true;
           
        }

        i++;
    }

    // ✅ SOLO UNA VEZ POR SLAVE
    if (slaveOk)
        modbusGuardOnSuccess(ctx);
    else if (slaveError)
        modbusGuardOnError(ctx);
}
