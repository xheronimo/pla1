#pragma once

#include "signal/signal_struct.h"
#include "i2c/i2c_chip_context.h"

// ==============================
// CONFIG
// ==============================
#define ENS160_MAX_CHIPS   8
#define ENS160_CACHE_MS   1000   // 1s

// ==============================
// OPCIONES (Signal.options)
// bit 0 → autocompensación T/RH
// bit 1 → modo IDLE (en vez de STANDARD)
// ==============================
#define ENS160_OPT_AUTOCOMP   0x01
#define ENS160_OPT_IDLE       0x02

// ==============================
// CANALES
// 0 → AQI
// 1 → TVOC (ppb)
// 2 → eCO2 (ppm)
// ==============================
bool ens160Init(uint8_t addr, uint8_t options = 0);
bool leerSignalENS160(const Signal& s, float& out);
bool ens160SetEnvironmentalData(uint8_t addr, float temp, float hum);
void ens160Reset();
