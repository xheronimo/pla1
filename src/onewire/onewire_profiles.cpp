#include "onewire_profiles.h"

OneWireProfile detectOneWireProfile(const uint8_t rom[8])
{
    switch (rom[0]) {
        case 0x28: return OneWireProfile::DS18B20;
        case 0x22: return OneWireProfile::DHT22;
        default:   return OneWireProfile::UNKNOWN;
    }
}