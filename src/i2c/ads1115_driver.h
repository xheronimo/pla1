#pragma once

#include <stdint.h>
#include "signal/signal_struct.h"
#include "chip/chip_context.h"

#define ADS1115_MAX_CHIPS 8
#define ADS1115_CACHE_MS  50     // refresco normal
#define ADS1115_WARMUP_MS 10     // tras init

bool leerSignalADS1115(const Signal& s, float& out);
void ads1115Reset();
