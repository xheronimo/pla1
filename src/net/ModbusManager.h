#pragma once
#include <stdint.h>

class ModbusManager
{
public:
    static bool init();

    static bool readHolding(
        uint8_t slave,
        uint16_t address,
        float& out
    );

    static bool readDiscrete(
        uint8_t slave,
        uint16_t address,
        bool& out
    );
};
