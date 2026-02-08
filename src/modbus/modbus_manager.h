#pragma once
#include <stdint.h>
#include <HardwareSerial.h>

class ModbusManager {
public:
    static void init(HardwareSerial* port, uint8_t dePin);

    static bool readHolding(
        uint8_t slave,
        uint16_t reg,
        uint16_t& out
    );

    static bool readInput(
        uint8_t slave,
        uint16_t reg,
        uint16_t& out
    );
};
