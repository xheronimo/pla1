#pragma once
#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>
#include "signal/signal_struct.h"

// ----------------------------
// CONFIG
// ----------------------------
constexpr uint8_t PCF_MAX_CHIPS = 8;
constexpr uint32_t PCF_CACHE_MS = 50;

// ----------------------------
// CACHE STRUCT
// ----------------------------
struct PcfCache {
    bool     valid;
    uint32_t lastReadMs;
    uint8_t  value;       // 8 bits = P0..P7
    uint8_t  errorCount;
};

// ----------------------------
// API
// ----------------------------

// Para ENTRADAS USUARIO
bool leerSignalPCF_User(const Signal& s, float& out);

// Para ENTRADAS SISTEMA
bool leerSignalPCF_System(const Signal& s, float& out);

// Inicialización (opcional)
void pcf8574ResetCaches();
