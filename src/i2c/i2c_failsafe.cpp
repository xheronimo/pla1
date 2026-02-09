#include "i2c/i2c_failsafe.h"
#include <Arduino.h>

#define CHIP_MAX_CONSEC_ERRORS  5
#define CHIP_MAX_TOTAL_ERRORS  20

bool chipRegisterError(ChipContext& ctx)
{
    ctx.errorCount++;
    ctx.consecutiveErrors++;

    if (ctx.consecutiveErrors >= CHIP_MAX_CONSEC_ERRORS)
    {
        ctx.state = ChipState::ERROR;
    }

    if (ctx.errorCount >= CHIP_MAX_TOTAL_ERRORS)
    {
        ctx.fatal = true;
        ctx.state = ChipState::ERROR;
        return false; // NO reintentar
    }

    return true; // se puede reintentar
}

void chipRegisterSuccess(ChipContext& ctx)
{
    ctx.consecutiveErrors = 0;
    ctx.lastReadTs = millis();
}
