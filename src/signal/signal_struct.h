// src/signal/signal_struct.h
#pragma once

#include <Arduino.h>
#include "signal_types.h"
#include "signal_source.h"

// ================================
// DEFINICIÓN DE SEÑAL
// ================================
struct SignalDef
{
    char id[16];              // ID único ("AI1", "D3", "MB_TEMP")
    SignalKind kind;          // DIGITAL / ANALOG
    SignalSource source;      // Origen físico (GPIO, I2C, MODBUS…)

    // Estado runtime
    float lastValue = 0;
    bool  valid     = false;
    uint32_t lastTs = 0;
};
