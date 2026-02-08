#pragma once
#include <stdbool.h>

void signalRegistryInit();
void signalRegistryPoll();
bool signalRegistryGet(const char* id, float& outValue);
