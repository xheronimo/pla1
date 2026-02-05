#pragma once
#include "analog_input_config.h"

class AnalogInput
{
public:
    AnalogInput(const AnalogInputConfig& cfg);

    void update(float rawValue);
    float getValue() const;
    void resetLatch();

private:
    AnalogInputConfig cfg;

    bool initialized = false;

    float filtered = 0.0f;
    float stable = 0.0f;
    float latchedValue = 0.0f;

    float scale(float raw) const;
};
