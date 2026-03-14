#pragma once
#include <stdint.h>
#include "i2c_chip.h"


#include <cstddef>

// Forward declaration
struct Signal;

// ================================
// METADATA
// ================================

struct ChipOptionDescriptor {
    const char* label;
    const char** values;
    uint8_t valueCount;
    uint8_t defaultIndex;
};

struct ChipMetadata {
    const char* name;
    uint8_t channelCount;

    ChipOptionDescriptor opt1;
    ChipOptionDescriptor opt2;
};

// ================================
// DRIVER INTERFACE
// ================================

typedef bool (*ChipDetectFn)(uint8_t addr);
typedef bool (*ChipInitFn)(uint8_t addr, uint8_t options);
typedef bool (*ChipReadFn)(const Signal& s, float& out);
typedef void (*ChipMetadataFn)(ChipMetadata& meta);
typedef bool (*ChipWriteFn)(const Signal& s, float value);   // 👈 NUEVO



struct I2CChipDriver
{
    I2CDevice type;

    ChipDetectFn detect;
    ChipInitFn init;
    ChipReadFn read;
    ChipWriteFn    write;
    ChipMetadataFn meta;

    bool allowAutoDetect;
};

const I2CChipDriver* i2cGetDriver(I2CDevice type);
const char* i2cGetDriverName(I2CDevice type);
bool i2cIsAddressConfigured(uint8_t addr);
size_t i2cGetDriverCount();
const I2CChipDriver* i2cGetDriverByIndex(size_t index);



