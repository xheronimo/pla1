#pragma once
#include <stdint.h>
enum class I2CDevice : uint8_t {
    NONE = 0,

    PCF8574,
    PCF8575,
    // Sensores ambientales
    SHT31,

    // ADC
    ADS1115,

    // Energía (futuro) V C P
    INA219,

    INA226,
//lumionosidad
    BH1750,

    //CO
    CCS811,
    ENS160,
    BME280,

    
    MAX
};