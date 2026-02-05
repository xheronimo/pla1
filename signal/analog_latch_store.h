#pragma once
#include <Arduino.h>

bool loadAnalogLatch(const String& id, float& value);
void saveAnalogLatch(const String& id, float value);
