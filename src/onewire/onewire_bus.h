#pragma once
#include <stdint.h>
#include <vector>
#include "onewire/onewire_device.h"

void onewireScanBus(uint8_t bus, std::vector<OneWireDevice>& out);