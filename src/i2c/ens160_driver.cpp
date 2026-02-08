#include "i2c/ens160_driver.h"

#define ENS160_REG_OPMODE  0x10
#define ENS160_REG_STATUS  0x20
#define ENS160_REG_DATA    0x30

static ChipContext ensCtx[ENS160_MAX_CHIPS];
static Ens160Cache ensCache[ENS160_MAX_CHIPS];

// ------------------------
static bool i2cRead(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t len)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(addr, len) != len)
        return false;

    for (uint8_t i = 0; i < len; i++)
        buf[i] = Wire.read();

    return true;
}

// ------------------------
static bool ens160Init(uint8_t addr)
{
    Wire.beginTransmission(addr);
    Wire.write(ENS160_REG_OPMODE);
    Wire.write(0x02); // standard mode
    return Wire.endTransmission() == 0;
}

// ------------------------
static bool ens160ReadAll(uint8_t addr, Ens160Cache& c)
{
    uint8_t buf[6];
    if (!i2cRead(addr, ENS160_REG_DATA, buf, 6))
        return false;

    c.tvoc = (buf[0] << 8) | buf[1];
    c.eco2 = (buf[2] << 8) | buf[3];
    c.aqi  = buf[4];

    c.valid = true;
    return true;
}

// ------------------------
void ens160Reset()
{
    for (uint8_t i = 0; i < ENS160_MAX_CHIPS; i++)
    {
        ensCtx[i] = {
            .state = ChipState::UNINITIALIZED,
            .initTs = 0,
            .lastReadTs = 0,
            .warmupMs = 60000,
            .retryMs = 5000,
            .errorCount = 0,
            .consecutiveErrors = 0,
            .busy = false
        };
        ensCache[i].valid = false;
    }
}

// ------------------------
static void ens160Update(uint8_t addr, ChipContext& ctx, Ens160Cache& c)
{
    uint32_t now = millis();

    switch (ctx.state)
    {
        case ChipState::UNINITIALIZED:
            if (ens160Init(addr))
            {
                ctx.state = ChipState::WARMUP;
                ctx.initTs = now;
            }
            break;

        case ChipState::WARMUP:
            if (now - ctx.initTs >= ctx.warmupMs)
                ctx.state = ChipState::READY;
            break;

        case ChipState::READY:
            if (ctx.busy) break;

            if (!ens160ReadAll(addr, c))
            {
                ctx.consecutiveErrors++;
                if (ctx.consecutiveErrors > 5)
                    ctx.state = ChipState::ERROR;
            }
            else
            {
                ctx.consecutiveErrors = 0;
                ctx.lastReadTs = now;
            }
            break;

        case ChipState::ERROR:
            if (now - ctx.lastReadTs > ctx.retryMs)
                ctx.state = ChipState::UNINITIALIZED;
            break;
    }
}

// ------------------------
bool leerSignalENS160(const Signal& s, float& out)
{
    uint8_t idx = s.address & 0x07;

    ens160Update(s.address, ensCtx[idx], ensCache[idx]);

    if (ensCtx[idx].state != ChipState::READY || !ensCache[idx].valid)
        return false;

    switch (s.channel)
    {
        case 0: out = ensCache[idx].tvoc; return true;
        case 1: out = ensCache[idx].eco2; return true;
        case 2: out = ensCache[idx].aqi;  return true;
        default: return false;
    }
}
