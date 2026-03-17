#pragma once
#include <stdint.h>

void i2cWatchdogInit();
void i2cWatchdogActivity();
void i2cWatchdogCheck();
void i2cBusReset();
bool i2cBusIsStuck();
