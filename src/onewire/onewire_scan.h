#pragma once
#include <vector>
#include "onewire_device.h"
#include <stdint.h>


void onewireScan(uint8_t gpio, std::vector<OneWireDevice>& out);