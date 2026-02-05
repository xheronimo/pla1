#pragma once
#include "signal_struct.h"

// Inicializa señales
void signalRegistryInit();

// Busca y evalúa una señal
bool signalGetValue(const String& id, float& outValue);
