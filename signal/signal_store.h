#pragma once
#include <stdint.h>

enum SignalType : uint8_t {
    SIG_ANALOG,
    SIG_DIGITAL,
    SIG_MODBUS
};

struct SignalValue {
    float value;
    bool  valid;
    uint32_t timestamp;
};


bool getSignalValue(const String& id, float& outValue);
void setSignalValue(const String& id, float value);
void clearSignal(const String& id);
void clearAllSignals();
