#pragma once
#include <stdint.h>

bool leerI2CDigital(uint8_t addr, uint8_t bit, bool& out);
bool leerI2CAnalog(uint8_t addr, uint8_t channel, float& out);
