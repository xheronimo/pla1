#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "signal/signal_struct.h"
#include "chip_context.h"

#define ENS160_MAX_CHIPS 8

struct Ens160Cache {
    bool  valid;
    float tvoc;
    float eco2;
    float aqi;
};

void ens160Reset();
bool leerSignalENS160(const Signal& s, float& out);
