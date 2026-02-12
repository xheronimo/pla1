#include "signal_read_modbus.h"
#include "modbus/modbus_manager.h"
#include "signal/signal_struct.h"



bool leerSignalModbus(const Signal& s, float& out)
{
    uint16_t regValue = 0;
    bool ok = false;

    switch (s.modbusType)
    {
        case ModbusRegType::MODBUS_HOLDING:
            ok = ModbusManager::readHolding(
                s.address,   // slave
                s.channel,   // register
                regValue
            );
            break;

        case ModbusRegType::MODBUS_INPUT:
            ok = ModbusManager::readInput(
                s.address,
                s.channel,
                regValue
            );
            break;

        default:
            return false;
    }

    if (!ok)
        return false;

    out = (float)regValue;
    return true;
}

