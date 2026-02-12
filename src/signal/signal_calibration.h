#pragma once
#include "signal_struct.h"

float calibrateValue(
    float raw,
    float rawMin,
    float rawMax,
    float realMin,
    float realMax
);

float applyCalibration(float raw, const Calibration& c);