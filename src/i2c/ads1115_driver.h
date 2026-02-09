#pragma once
#include <stdint.h>
#include "signal/signal_struct.h"
#include "i2c/i2c_chip_context.h"

// ==============================
// CONFIG
// ==============================
#define ADS1115_MAX_CHIPS   8
#define ADS1115_CACHE_MS   300

// ==============================
// OPCIONES
// ==============================
enum class Ads1115Gain : uint8_t {
    GAIN_6_144V = 0,
    GAIN_4_096V,
    GAIN_2_048V,
    GAIN_1_024V,
    GAIN_0_512V,
    GAIN_0_256V
};

enum class Ads1115Rate : uint8_t {
    SPS_8   = 0,
    SPS_16,
    SPS_32,
    SPS_64,
    SPS_128,
    SPS_250,
    SPS_475,
    SPS_860
};

struct Ads1115Options {
    Ads1115Gain gain;
    Ads1115Rate rate;
};
const Ads1115Options Ads1115Default ={Ads1115Gain::GAIN_4_096V, Ads1115Rate::SPS_128};
// ==============================
// API
// ==============================
bool ads1115Init(uint8_t addr, const Ads1115Options& opt = Ads1115Default );
bool leerSignalADS1115(const Signal& s, float& out);
void ads1115Reset();
