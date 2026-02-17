#include "signal_read_i2c.h"
#include <Arduino.h>

#include "i2c/i2c_chip_registry.h"
#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_bus.h"

bool leerSignalI2C(const Signal& s, float& out)
{
    if (s.bus != BusType::BUS_I2C)
        return false;

    // 🔐 Punto único de control
    ChipContext* ctx = i2cGetDriver(s.chip, s.address);
    if (!ctx || !ctx->driver || !ctx->driver->read)
        return false;

    bool ok = ctx->driver->read(s, out);

    if (ok)
    {
        ctx->lastReadTs = millis();
        ctx->consecutiveErrors = 0;
        ctx->totalReads++;
    }
    else
    {
        ctx->consecutiveErrors++;
        ctx->totalErrors++;
    }

    return ok;
}

