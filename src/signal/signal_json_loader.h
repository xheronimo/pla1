#pragma once

#include <ArduinoJson.h>
#include "signal_struct.h"

bool loadSignalsFromJson(const JsonArrayConst &arr);
Signal* signalManagerFindById(const char* id);
