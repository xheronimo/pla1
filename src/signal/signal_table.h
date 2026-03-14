#pragma once
#include "signal_struct.h"

#define MAX_SIGNALS 64

extern Signal signalTable[MAX_SIGNALS];
extern uint8_t signalCount;
