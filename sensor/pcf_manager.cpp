#include "pcf_manager.h"
#include <Wire.h>

static uint8_t pcfAddr[4] = {0x20, 0x21, 0x22, 0x23};

bool PCF_Read(uint8_t chip, uint8_t pin)
{
    if (chip >= 4 || pin > 7)
        return false;

    Wire.beginTransmission(pcfAddr[chip]);
    Wire.endTransmission();
    Wire.requestFrom(pcfAddr[chip], (uint8_t)1);

    if (!Wire.available())
        return false;

    uint8_t v = Wire.read();
    return (v & (1 << pin)) != 0;
}
