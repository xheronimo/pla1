#pragma once
#include <Arduino.h>

bool loadLatch(const String& id);
void saveLatch(const String& id, bool value);
void clearAllLatches();
