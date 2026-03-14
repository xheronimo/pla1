#pragma once
#include <stdint.h>
#include <cstddef>

struct OneWireMeta {
    uint8_t family;
    const char* name;
    bool hasTemp;
    bool hasHum;
};

const OneWireMeta* onewireGetMeta(uint8_t family);