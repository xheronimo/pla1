#pragma once
#include <Arduino.h>

bool getSignalValue(const String& src, float& outValue);
#pragma once
#include "signal_struct.h"

// Usada por reglas, alarmas, expresiones
bool getSignalValue(const char* signalId, float& outValue);
bool getSignalState(const char* signalId, bool& outState);
