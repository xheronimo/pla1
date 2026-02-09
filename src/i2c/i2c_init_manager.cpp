
#include "i2c_init_manager.h"


extern ChipContext chipCtx[I2CDevice::MAX][8];

void i2cInitConfiguredChips(const Signal* signals, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        const Signal& s = signals[i];

        if (s.bus != BusType::BUS_I2C)
            continue;

        uint8_t idx = s.address & 0x07;
        ChipContext& ctx = chipCtx[(int)s.chip][idx];

        // Ya inicializado
        if (ctx.state != ChipState::UNINITIALIZED)
            continue;

        // ¿Existe físicamente?
        if (!i2cDetect(s.address))
        {
            ctx.state = ChipState::ERROR;
            continue;
        }

        // Buscar init en la tabla
        for (size_t t = 0; t < I2C_CHIP_TABLE_SIZE; t++)
        {
            if (i2cChipTable[t].chip == s.chip)
            {
                ctx.state    = ChipState::INITIALIZING;
                ctx.initTs   = millis();
                ctx.warmupMs = i2cChipTable[t].warmupMs;
                ctx.retryMs  = i2cChipTable[t].retryMs;

                // Llamada NO BLOQUEANTE
                i2cChipTable[t].initFn(s.address, s.options);
                break;
            }
        }
    }
}

vvoid i2cInitFromSignals()
{
    i2cChipRegistryInit();

    for (size_t i = 0; i < signalCount; i++)
    {
        const Signal& s = signalTable[i];

        if (s.bus == BusType::BUS_I2C)
        {
            i2cRegisterChip(s.bus, s.chip, s.address);
        }
    }
}

