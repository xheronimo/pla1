#pragma once
#include "signal/signal_struct.h"

// Escritura genérica de señales (solo actuadores)
bool signalWrite(Signal& s, float value);

static bool writeGpioSignal(const Signal& s, float value);

bool writeSignalById(const char* id, float value);
bool escribirSignal(const Signal& s, float value);