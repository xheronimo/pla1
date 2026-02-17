#include "signal_calibration.h"
#include <math.h>

float calibrateValue(
    float raw,
    float rawMin,
    float rawMax,
    float realMin,
    float realMax)
{
    if (rawMax == rawMin)
        return realMin;

    if (raw < rawMin)
        raw = rawMin;
    if (raw > rawMax)
        raw = rawMax;

    return realMin +
           (raw - rawMin) * (realMax - realMin) /
               (rawMax - rawMin);
}

float applyCalibration(float raw,
                       Calibration& calib,
                       bool& stable,
                       bool& clamped)
{
    float value = raw;
    stable = true;
    clamped = false;

    // ===============================
    // 1️⃣ Escalado lineal
    // ===============================
    if (calib.rawMax != calib.rawMin)
    {
        float ratio =
            (raw - calib.rawMin) /
            (calib.rawMax - calib.rawMin);

        value =
            calib.realMin +
            ratio * (calib.realMax - calib.realMin);
    }

    // ===============================
    // 2️⃣ Offset
    // ===============================
    value += calib.offset;

    // ===============================
    // 3️⃣ Clamp
    // ===============================
    if (calib.clamp)
    {
        if (value < calib.realMin)
        {
            value = calib.realMin;
            clamped = true;
        }
        if (value > calib.realMax)
        {
            value = calib.realMax;
            clamped = true;
        }
    }

    // ===============================
    // 4️⃣ Histéresis de medida
    // ===============================
    if (calib.measureHysteresis > 0.0f)
    {
        if (!calib.hasStableValue)
        {
            calib.lastStableValue = value;
            calib.hasStableValue = true;
        }
        else if (fabs(value - calib.lastStableValue) <
                 calib.measureHysteresis)
        {
            stable = false;
            return calib.lastStableValue;
        }
        else
        {
            calib.lastStableValue = value;
        }
    }

    // ===============================
    // 5️⃣ Filtro EMA
    // ===============================
    if (calib.emaAlpha > 0.0f)
    {
        if (!calib.emaInit)
        {
            calib.emaValue = value;
            calib.emaInit = true;
        }
        else
        {
            calib.emaValue =
                calib.emaAlpha * value +
                (1.0f - calib.emaAlpha) * calib.emaValue;
        }

        value = calib.emaValue;
    }

    return value;
}

