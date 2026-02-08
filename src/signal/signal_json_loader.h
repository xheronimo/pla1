#pragma once

#include <ArduinoJson.h>
#include "bus/bus_struct.h"

bool loadSignalsFromJson(const JsonArrayConst& arr);
static BusType parseBus(const char* s);
static SignalKind parseKind(const char* s);