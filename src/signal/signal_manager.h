#pragma once
#include <stddef.h>
#include "signal_struct.h"

void   signalManagerInit();
bool   signalManagerAdd(const Signal& s);

size_t signalManagerCount();
Signal* signalManagerGet(size_t index);

void   signalManagerPollAll();
bool   signalManagerGetValue(const char* id, float& outValue);

bool signalManagerGetValueById(uint32_t id, float& outValue);
bool signalManagerGetErrorById(uint32_t id, bool& outError);


bool signalPhysicalExists(const Signal& s, const char* ignoreId);

Signal* signalManagerGetAll(size_t& count);
size_t signalManagerGetCount();

Signal* signalManagerGetById(const char* id);