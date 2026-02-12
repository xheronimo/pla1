#pragma once
#include "signal_struct.h"

SignalKind parseSignalKind(const char* str);
BusType    parseBusType(const char* str);
I2CDevice  parseI2CDevice(const char* str);

