#pragma once
#include <stdint.h>
#include "i2c_chip_context.h"
#include "signal/signal_struct.h"

#define MAX_I2C_CHIPS 16

struct I2CChipEntry {
    BusType     bus;
    I2CDevice   chip;
    uint8_t     address;
    ChipContext ctx;
};

void i2cChipRegistryInit();
ChipContext* i2cGetChipContext(BusType bus, I2CDevice chip, uint8_t addr);
void i2cRegisterChip(BusType bus, I2CDevice chip, uint8_t addr);
uint8_t i2cGetChipCount();
I2CChipEntry* i2cGetAllChips();
