#include "i2c_chip_registry.h"

// INIT reales (ya existen en tus drivers)
bool ads1115Init(uint8_t addr, uint8_t opt);
bool ina219Init(uint8_t addr, uint8_t opt);
bool sht31Init(uint8_t addr, uint8_t opt);
bool ens160Init(uint8_t addr, uint8_t opt);

// Tabla central
const I2CChipInitEntry i2cChipTable[] = {
    { I2CDevice::ADS1115, ads1115Init,  2,     1000 },
    { I2CDevice::INA219,  ina219Init,   10,    1000 },
    { I2CDevice::SHT31,   sht31Init,    15,    1000 },
    { I2CDevice::ENS160,  ens160Init,   60000, 5000 },
};

const size_t I2C_CHIP_TABLE_SIZE =
    sizeof(i2cChipTable) / sizeof(i2cChipTable[0]);


#include "i2c/i2c_chip_registry.h"

static I2CChipEntry table[MAX_I2C_CHIPS];
static uint8_t chipCount = 0;

void i2cChipRegistryInit()
{
    chipCount = 0;
}

void i2cRegisterChip(BusType bus, I2CDevice chip, uint8_t addr)
{
    // evitar duplicados
    for (uint8_t i = 0; i < chipCount; i++)
    {
        if (table[i].bus == bus &&
            table[i].chip == chip &&
            table[i].address == addr)
            return;
    }

    if (chipCount >= MAX_I2C_CHIPS)
        return;

    table[chipCount++] = {
        bus,
        chip,
        addr,
        {}   // ChipContext vacío
    };
}

ChipContext* i2cGetChipContext(BusType bus, I2CDevice chip, uint8_t addr)
{
    for (uint8_t i = 0; i < chipCount; i++)
    {
        if (table[i].bus == bus &&
            table[i].chip == chip &&
            table[i].address == addr)
        {
            return &table[i].ctx;
        }
    }
    return nullptr;
}

uint8_t i2cGetChipCount()
{
    return chipCount;
}

I2CChipEntry* i2cGetAllChips()
{
    return table;
}
    