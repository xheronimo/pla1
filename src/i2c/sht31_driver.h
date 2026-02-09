#pragma once
#include <stdint.h>
#include "signal/signal_struct.h"
#include "i2c/i2c_chip_context.h"

// -----------------------------
// CONFIG
// -----------------------------
#define SHT31_MAX_CHIPS  8
#define SHT31_CACHE_MS   500

// -----------------------------
// OPCIONES
// -----------------------------
enum class Sht31Repeatability : uint8_t {
    LOW,
    MEDIUM,
    HIGH
};

struct Sht31Options {
    Sht31Repeatability repeatability;
    bool heater;
};
const Sht31Options SHT31_DEFAULT_OPT = { Sht31Repeatability::HIGH, false };
// -----------------------------
// API
// -----------------------------
bool sht31Init(uint8_t addr, const Sht31Options& opt= SHT31_DEFAULT_OPT);
bool leerSignalSHT31(const Signal& s, float& out);
void sht31Reset();
