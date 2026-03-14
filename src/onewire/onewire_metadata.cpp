#include "onewire_metadata.h"

static const OneWireMeta OW_TYPES[] = {
    { 0x28, "DS18B20",      true,  false },
    { 0x3A, "OW_TEMP_HUM",  true,  true  },
};

const OneWireMeta* onewireGetMeta(uint8_t family)
{
    for (auto& t : OW_TYPES) {
        if (t.family == family)
            return &t;
    }
    return nullptr;
}