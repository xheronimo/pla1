#pragma once
#include <stdint.h>

// Digital
bool leerPCFBit(uint8_t addr, uint16_t bit);
bool leerI2CDigital(uint8_t addr, uint16_t ch);
bool leerModbusDigital(uint8_t slave, uint16_t reg);

// Analógico
float leerI2CAnalogico(uint8_t addr, uint16_t ch);
float leerModbusAnalogico(uint8_t slave, uint16_t reg);
