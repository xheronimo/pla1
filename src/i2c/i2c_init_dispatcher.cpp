#include "i2c/i2c_init_dispatcher.h"
#include "i2c/i2c_chip_registry.h"

// INIT reales (los tuyos)
bool ads1115Init(uint8_t addr, uint8_t opt);
bool ina219Init(uint8_t addr, uint8_t opt);
bool sht31Init(uint8_t addr, uint8_t opt);
bool ens160Init(uint8_t addr, uint8_t opt);

// TABLA CENTRAL
static const I2CChipInitEntry i2cChipTable[] = {
    { I2CDevice::ADS1115, ads1115Init,  2,     1000 },
    { I2CDevice::INA219,  ina219Init,   10,    1000 },
    { I2CDevice::SHT31,   sht31Init,    15,    1000 },
    { I2CDevice::ENS160,  ens160Init,   60000, 5000 },
};

static const size_t I2C_CHIP_TABLE_SIZE =
    sizeof(i2cChipTable) / sizeof(i2cChipTable[0]);

// --------------------------------------------
// INIT GENÉRICO
// --------------------------------------------
bool i2cInitChip(const Signal& s)
{
    ChipContext* cc = i2cGetChipContext(
        s.bus,
        s.chip,
        s.address
    );

    if (!cc)
        return false;

    for (size_t i = 0; i < I2C_CHIP_TABLE_SIZE; i++)
    {
        const auto& e = i2cChipTable[i];
        if (e.chip != s.chip)
            continue;

        cc->state    = ChipState::INITIALIZING;
        cc->initTs   = millis();
        cc->warmupMs = e.warmupMs;
        cc->retryMs  = e.retryMs;

        bool ok = e.initFn(s.address, s.options);

        if (!ok)
        {
            cc->state = ChipState::ERROR;
            return false;
        }

        cc->state = ChipState::WARMUP;
        return true;
    }

    // Chip sin init especial
    cc->state = ChipState::READY;
    return true;
}
