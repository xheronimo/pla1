#pragma once
#include "signal_struct.h"
#include "i2c/i2c_chip.h"


bool readSignalI2C(const Signal& s, float& out);
