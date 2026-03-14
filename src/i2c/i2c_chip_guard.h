#pragma once
#include "i2c_chip_context.h"

bool i2cChipGuardBeforeRead(ChipContext* ctx, uint32_t now);
void i2cChipGuardOnError(ChipContext* ctx);
void i2cChipGuardOnSuccess(ChipContext* ctx, uint32_t now);

void i2cTryRecoverBus(ChipContext* ctx);
static void i2cPulseClock();