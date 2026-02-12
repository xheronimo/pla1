#include "i2c_chip_context.h"

#define I2C_MAX_TYPES 16
#define I2C_MAX_CHIPS_PER_TYPE 8
static ChipContext chipCtx[I2C_MAX_TYPES][I2C_MAX_CHIPS_PER_TYPE];

ChipContext* i2cGetChipContext(I2CDevice type, uint8_t addr)
{
    uint8_t idx = addr & 0x07;
    return &chipCtx[(int)type][idx];
}

void i2cResetAllContexts()
{
    for (int t = 0; t < I2C_MAX_TYPES; t++)
        for (int i = 0; i < I2C_MAX_CHIPS_PER_TYPE; i++)
            chipCtx[t][i] = ChipContext();
}
