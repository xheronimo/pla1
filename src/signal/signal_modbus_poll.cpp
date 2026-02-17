#include "signal_modbus_poll.h"
#include "signal/signal_struct.h"
#include "modbus/modbus_manager.h"
#include "modbus/modbus_config.h"
#include "vector"

extern Signal signalTable[];
extern size_t signalCount;

void modbusPollSignals()
{
    for (uint8_t slave = 1; slave <= MODBUS_MAX_SLAVES; slave++)
    {
        // Recoger señales de este slave
        std::vector<Signal *> list;

        for (size_t i = 0; i < signalCount; i++)
        {
            Signal &s = signalTable[i];

            if (s.bus == BusType::BUS_MODBUS &&
                s.address == slave &&
                !s.writable)
            {
                list.push_back(&s);
            }
        }

        if (list.empty())
            continue;

        // Ordenar por registro
        std::sort(list.begin(), list.end(),
                  [](Signal *a, Signal *b)
                  {
                      return a->channel < b->channel;
                  });

        // Agrupar bloques contiguos
        size_t i = 0;

        while (i < list.size())
        {
            uint16_t startReg = list[i]->channel;
            uint8_t count = 1;

            while (i + count < list.size() &&
                   list[i + count]->channel ==
                       startReg + count &&
                   count < MODBUS_MAX_BLOCK_SIZE)
            {
                count++;
            }

            uint16_t buffer[MODBUS_MAX_BLOCK_SIZE];

            if (ModbusManager::readBlock(
                    slave,
                    list[i]->modbusType,
                    startReg,
                    count,
                    buffer))
            {
                for (uint8_t k = 0; k < count; k++)
                {
                    list[i + k]->raw = buffer[k];
                    list[i + k]->value = buffer[k];
                    list[i + k]->valid = true;
                }
            }

            i += count;
        }
    }
}
