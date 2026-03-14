#pragma once
#include <stdint.h>

enum class OneWireProfile : uint8_t {
    UNKNOWN,
    DS18B20,
    DHT22
};


// ------------------------------
// Metadata por tipo OneWire
// ------------------------------


struct OneWireReading {
    float temperature;
    float humidity;
    bool  hasHumidity;
};



OneWireProfile detectOneWireProfile(const uint8_t rom[8]);