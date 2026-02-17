#include "i2c_signal_factory.h"
#include "signal/signal_manager.h"
#include "i2c/i2c_chip_registry.h"
#include <Arduino.h>

bool createSignalsForChip(I2CDevice type,
                          uint8_t addr,
                          const std::vector<uint8_t>& channels)
{
    const I2CChipDriver* drv = i2cGetDriver(type);
    if (!drv)
        return false;

    ChipMetadata meta;
    drv->meta(meta);

    for (uint8_t ch : channels)
    {
        if (ch >= meta.channelCount)
            continue;

        Signal s{};
        memset(&s, 0, sizeof(Signal));

        char idBuf[32];
        snprintf(idBuf, sizeof(idBuf),
                 "%s_%02X_%u",
                 meta.name, addr, ch);

        s.id = strdup(idBuf);

        s.kind = SignalKind::SENSOR_ANALOG;
        s.bus  = BusType::BUS_I2C;
        s.chip = type;

        s.address = addr;
        s.channel = ch;
        s.options = 0;

        s.systemReserved = false;
        s.lockedConfig   = false;
        s.writable       = false;

        if (!signalManagerAdd(s))
            return false;
    }

    return true;
}
