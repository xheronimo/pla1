#include "onewire_scan.h"
#include "onewire_metadata.h"
#include <OneWire.h>
#include <cstring>

void onewireScan(uint8_t gpio, std::vector<OneWireDevice>& out)
{
    OneWire ow(gpio);
    uint8_t rom[8];

    ow.reset_search();
    while (ow.search(rom)) {
        OneWireDevice d{};
        memcpy(d.rom, rom, 8);
        d.busPin = gpio;
        d.family = rom[0];
        out.push_back(d);
    }
}