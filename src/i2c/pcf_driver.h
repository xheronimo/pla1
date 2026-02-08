#pragma once
#include "signal/signal_struct.h"
#include "i2c_chip_context.h"

#define PCF_MAX_CHIPS 8
#define PCF_CACHE_MS  100

bool leerSignalPCF_User(const Signal& s, float& out);
bool leerSignalPCF_System(const Signal& s, float& out);
void pcfReset();

//