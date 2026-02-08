#include "signal_calibration.h"

float calibrateValue(
    float raw,
    float rawMin,
    float rawMax,
    float realMin,
    float realMax
)
{
    if (rawMax == rawMin)
        return realMin;

    if (raw < rawMin) raw = rawMin;
    if (raw > rawMax) raw = rawMax;

    return realMin +
           (raw - rawMin) * (realMax - realMin) /
           (rawMax - rawMin);
}


static float applyCalibration(float raw, const Calibration& c)
{
    if (c.rawMax == c.rawMin)
        return c.realMin;

    float val = c.realMin +
        (raw - c.rawMin) *
        (c.realMax - c.realMin) /
        (c.rawMax - c.rawMin);

    if (c.clamp)
    {
        if (val < c.realMin) val = c.realMin;
        if (val > c.realMax) val = c.realMax;
    }

    return val;
}

