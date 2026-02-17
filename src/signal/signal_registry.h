#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "signal_struct.h"

void signalRegistryInit();
void signalRegistryPoll();
bool signalRegistryGet(const char* id, float& outValue);
size_t signalRegistryGetCount();
Signal* signalRegistryGetAll(size_t& count);
