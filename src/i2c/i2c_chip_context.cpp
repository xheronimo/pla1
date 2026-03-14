#include "i2c_chip_context.h"
#include "system/LogSystem.h"

#define I2C_MAX_TYPES 16
#define I2C_MAX_CHIPS_PER_TYPE 8
static ChipContext chipCtx[I2C_MAX_TYPES][I2C_MAX_CHIPS_PER_TYPE];


ChipContext* i2cGetChipContext(I2CDevice type, uint8_t addr)
{
    for (int i = 0; i < I2C_MAX_CHIPS_PER_TYPE; i++)
    {
        ChipContext& ctx = chipCtx[(int)type][i];

        if (ctx.allocated && ctx.address == addr)
            return &ctx;
    }

    // 🔧 no existe → asignamos slot libre
    for (int i = 0; i < I2C_MAX_CHIPS_PER_TYPE; i++)
    {
        ChipContext& ctx = chipCtx[(int)type][i];

        if (!ctx.allocated)
        {
            ctx = ChipContext();   // reset limpio
            ctx.address = addr;
            ctx.type = type;
            ctx.allocated = true;
            ctx.inUse = true;
            return &ctx;
        }
    }

    // ❌ sin slots disponibles
    escribirLog("I2C: No hay slot para chip %u addr 0x%02X",
                (uint8_t)type, addr);
    return nullptr;
}

void i2cResetAllContexts()
{
    for (int t = 0; t < I2C_MAX_TYPES; t++)
        for (int i = 0; i < I2C_MAX_CHIPS_PER_TYPE; i++)
        {
            chipCtx[t][i] = ChipContext();
            chipCtx[t][i].allocated = false;
        }
}

float i2cChipHealth(const ChipContext* ctx)
{
    if (!ctx || ctx->totalReads == 0)
        return 100.0f;

    float errRate = (float)ctx->totalErrors / (float)ctx->totalReads;
    float health = 100.0f - errRate * 100.0f;
    if (health < 0) health = 0;
    return health;
}
