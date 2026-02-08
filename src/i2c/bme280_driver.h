#pragma once

#include <stdint.h>
#include <Arduino.h>
#include "signal/signal_struct.h"

// ==============================
// CONFIG
// ==============================
#define BME280_MAX_CHIPS   8
#define BME280_CACHE_MS   1000   // BME no necesita refresco rápido

// ==============================
// CACHE
// ==============================
struct Bme280Cache {
    bool     valid;
    uint32_t lastReadMs;
    float    temperature;  // °C
    float    humidity;     // %
    float    pressure;     // hPa
};

// ==============================
// API
// ==============================
void bme280ResetCaches();
bool leerSignalBME280(const Signal& s, float& out);
