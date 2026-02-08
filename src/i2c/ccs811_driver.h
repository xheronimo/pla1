#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "signal/signal_struct.h"

// -----------------------------
#define CCS811_MAX_CHIPS     4
#define CCS811_CACHE_MS      2000
#define CCS811_WARMUP_MS     (20UL * 60UL * 1000UL) // 20 minutos

// -----------------------------
enum class CCS811Channel : uint8_t {
    ECO2 = 0,
    TVOC = 1
};

// -----------------------------
struct CCS811Cache {
    bool     present;
    bool     ready;
    bool     warming;
    uint32_t startMs;
    uint32_t lastReadMs;

    uint16_t eco2;
    uint16_t tvoc;

    uint8_t  errorCount;
};

// -----------------------------
void ccs811ResetCaches();
bool leerSignalCCS811(const Signal& s, float& out);
