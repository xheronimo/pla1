#pragma once

#include "signal/signal_struct.h"
#include "i2c/i2c_chip_context.h"

// ==============================
// CONFIG
// ==============================
#define BME280_MAX_CHIPS   8
#define BME280_CACHE_MS   1000

// ==============================
// CANALES
// ==============================
// channel:
// 0 → Temperatura (°C)
// 1 → Humedad (%)
// 2 → Presión (hPa)

// ==============================
// OPCIONES (Signal.options)
// ==============================
// bits 0..1 → oversampling
// 0 = x1
// 1 = x2
// 2 = x4
// 3 = x8
// ==============================
bool bme280Init(uint8_t addr, uint8_t options = 0);
bool leerSignalBME280(const Signal& s, float& out);
void bme280Reset();
