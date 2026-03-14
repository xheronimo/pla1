#pragma once
#include "onewire_device.h"
#include "onewire_metadata.h"



bool onewireReadTemperature(uint8_t busPin, const uint8_t rom[8], float& out);
bool onewireReadHumidity(uint8_t busPin, const uint8_t rom[8], float& out);