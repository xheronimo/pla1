#include "i2c_init_from_signals.h"
#include "i2c_chip_registry.h"
#include "signal/signal_manager.h"
#include "i2c_chip_context.h"
#include "system/LogSystem.h"
#include <Arduino.h>

void i2cInitFromSignals()
{
    size_t count = signalManagerCount();

    for (size_t i = 0; i < count; i++)
    {
        Signal *s = signalManagerGet(i);
        if (!s)
            continue;

        if (s->bus != BusType::BUS_I2C)
            continue;

        ChipContext *ctx = i2cGetChipContext(s->chip, s->address);
        if (!ctx)
            continue;

        ctx->inUse = true;

        // 🆕 CONFIGURACIÓN NUEVA O CAMBIADA
        if (!ctx->optionsSet || ctx->options != s->options)
        {
            ctx->options = s->options;
            ctx->optionsSet = true;

            // 🔄 RESET CONTROLADO DEL CHIP
            ctx->state = ChipState::STATE_UNINITIALIZED;
            ctx->consecutiveErrors = 0;
            ctx->errorCount = 0;

            escribirLog(
                "I2C: Config cambiada → reset chip %u addr 0x%02X",
                
                (uint8_t)s->chip,
                ctx->address);
        }
        if (ctx->options != s->options && ctx->state == ChipState::STATE_READY)
        {
            escribirLog("I2C: Chip activo reconfigurado → reset seguro");
        }
    }
}
