#include "signal_read_modbus.h"
#include "signal/signal_struct.h"
#include "modbus/modbus_manager.h"
#include "modbus/modbus_guard.h"
#include "modbus/modbus_decode.h"
#include "modbus/modbus_utils.h"

bool leerSignalModbus(const Signal &s, float &out)
{
    uint8_t words = modbusNormalizeWordCount(s);
    uint16_t buffer[2];

    bool ok = false;

    if (s.modbusType == ModbusRegType::MODBUS_HOLDING)
    {
        ok = ModbusManager::readHoldingRaw(
            s.address,
            s.channel,
            words,
            buffer);
    }
    else if (s.modbusType == ModbusRegType::MODBUS_INPUT)
    {
        ok = ModbusManager::readInputRaw(
            s.address,
            s.channel,
            words,
            buffer);
    }
    else
    {
        return false;
    }

    if (!ok)
        return false;

    if (!modbusDecodeValue(buffer, s, out))
    {
        ModbusSlaveContext *ctx = modbusGetContext(s.address);
        if (ctx)
        {
            ctx->configErrors++;
            modbusCheckConfigFault(ctx);
        }
        return false;
    }
}