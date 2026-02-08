#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "signal/signal_struct.h"

// -----------------------------
#define PCF8575_MAX_CHIPS   8
#define PCF8575_CACHE_MS    100

// -----------------------------
struct Pcf8575Cache {
    bool     valid;
    uint32_t lastReadMs;
    uint16_t value;       // 16 bits
    uint8_t  errorCount;
};

// -----------------------------
void pcf8575ResetCaches();
bool leerSignalPCF8575(const Signal& s, float& out);
