#pragma once
#include <stdint.h>

bool     systemHasCriticalFault();
void     systemAcknowledgeFault();
uint32_t systemFaultDurationMs();
void     systemFaultTick();

extern bool g_safeMode;

void systemFaultInit();