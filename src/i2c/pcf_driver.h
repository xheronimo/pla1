#pragma once
#include "signal/signal_struct.h"

// ------------------------------
// CONFIG
// ------------------------------
#define PCF_MAX_CHIPS   8
#define PCF_CACHE_MS   50
#define PCF_MAX_ERRORS 5

// ------------------------------
// CACHE
// ------------------------------
struct PcfCache {
    bool     valid = false;
    uint16_t value = 0;      // estado completo I/O
    uint32_t lastReadMs = 0;
    uint8_t  errorCount = 0;
};

// ------------------------------
// API ENTRADAS
// ------------------------------
bool leerSignalPCF_User(const Signal& s, float& out);
bool leerSignalPCF_System(const Signal& s, float& out);

// ------------------------------
// API SALIDAS
// ------------------------------
bool escribirSignalPCF_User(const Signal& s, float value);
bool escribirSignalPCF_System(const Signal& s, float value);

// ------------------------------
void pcfResetCaches();
