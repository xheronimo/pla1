#pragma once
#include <stdbool.h>
#include <stdint.h>

extern bool g_lastResetPower;
extern bool g_lastResetSoftware;
extern bool g_lastResetWdt;
extern uint32_t g_wdtResetCount;

void logBootReason();