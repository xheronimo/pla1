#include "i2c_init_from_signals.h"
#include "i2c_chip_registry.h"
#include "signal/signal_manager.h"
#include "i2c_chip_context.h"
#include <Arduino.h>

void i2cInitFromSignals()
{
    size_t count = signalManagerCount();

for (size_t i = 0; i < count; i++)
{
    Signal* s = signalManagerGet(i);
    if (!s) continue;

        if (s->bus != BusType::BUS_I2C)
            continue;

        const I2CChipDriver* drv = i2cGetDriver(s->chip);
        if (!drv)
            continue;

        ChipContext* ctx = i2cGetChipContext(s->chip, s->address);
        if (!ctx)
            continue;

        if (ctx->state != ChipState::UNINITIALIZED)
            continue;

        ctx->state = ChipState::INITIALIZING;
        ctx->initTs = millis();
        ctx->warmupMs = 0;
        ctx->retryMs  = 1000;
        ctx->errorCount = 0;
        ctx->consecutiveErrors = 0;

        drv->init(s->address, s->options);
    }
}
