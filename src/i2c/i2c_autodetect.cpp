#include "i2c_autodetect.h"
#include "i2c_bus.h"

std::vector<I2CDetectedChip> i2cAutoDetect()
{
    std::vector<I2CDetectedChip> found;

    for (uint8_t addr = 1; addr < 127; addr++)
    {
        if (!i2cDetect(addr))
            continue;

        for (size_t i = 0; i < i2cGetDriverCount(); i++)
        {
            const I2CChipDriver* drv = i2cGetDriverByIndex(i);
            if (!drv || !drv->allowAutoDetect)
                continue;

            if (drv->detect && drv->detect(addr))
            {
                found.push_back({ drv->type, addr });
            }
        }
    }

    return found;
}
