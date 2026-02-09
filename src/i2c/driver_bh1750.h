#pragma once

#include "signal/signal_struct.h"
#include "i2c/i2c_chip_context.h"

// ==============================
// CONFIG
// ==============================
#define BH1750_MAX_CHIPS   8
#define BH1750_CACHE_MS   300   // lectura rápida

// ==============================
// OPCIONES (Signal.options)
// ==============================
// bits 0..1 → modo de medida
// ==============================
enum class Bh1750Mode : uint8_t {
    CONT_HIGH_RES  = 0, // 1 lx / 120ms
    CONT_HIGH_RES2 = 1, // 0.5 lx / 120ms
    CONT_LOW_RES   = 2  // 4 lx / 16ms
};

// API
bool bh1750Init(uint8_t addr, Bh1750Mode mode = Bh1750Mode::CONT_HIGH_RES);
bool leerSignalBH1750(const Signal& s, float& out);
void bh1750Reset();
