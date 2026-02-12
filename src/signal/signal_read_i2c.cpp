#include "signal_read_i2c.h"
#include <Arduino.h>

#include "i2c/i2c_chip_registry.h"
#include "i2c/i2c_chip_context.h"

bool leerSignalI2C(const Signal& s, float& out)
{
    if (s.bus != BusType::BUS_I2C)
        return false;

    const I2CChipDriver* drv = i2cGetDriver(s.chip);
    if (!drv || !drv->read)
        return false;

    ChipContext* ctx = i2cGetChipContext(s.chip, s.address);
    if (!ctx)
        return false;

    if (ctx->state != ChipState::READY)
        return false;

    bool ok = drv->read(s, out);

    if (ok)
    {
        ctx->lastReadTs = millis();
        ctx->consecutiveErrors = 0;
    }
    else
    {
        ctx->errorCount++;
        ctx->consecutiveErrors++;
    }

    return ok;
}
