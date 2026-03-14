// onewire_bus.cpp
#include "onewire_bus.h"
#include "onewire/onewire_device.h"
#include <OneWire.h>

void onewireScanBus(uint8_t bus, std::vector<OneWireDevice>& out)
{
    OneWire ow(bus);
    uint8_t rom[8];

    ow.reset_search();
    while (ow.search(rom)) {
        OneWireDevice d{};
        memcpy(d.rom, rom, 8);
        d.busPin = bus;
        out.push_back(d);
    }
}