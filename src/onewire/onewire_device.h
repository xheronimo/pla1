#pragma once
#include <stdint.h>

struct OneWireDevice {
    uint8_t rom[8];     // ROM completa
    uint8_t family;     // rom[0]
    uint8_t busPin;     // GPIO del bus


    //bool hasTemp;
    //bool hasHum;
};