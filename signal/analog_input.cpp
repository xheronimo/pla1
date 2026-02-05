#include "analog_input.h"

AnalogInput::AnalogInput(const AnalogInputConfig& c)
    : cfg(c)
{
}

float AnalogInput::scale(float raw) const
{
    if (cfg.rawMax == cfg.rawMin)
        return cfg.engMin;

    float norm = (raw - cfg.rawMin) / (cfg.rawMax - cfg.rawMin);
    return cfg.engMin + norm * (cfg.engMax - cfg.engMin);
}

void AnalogInput::update(float raw)
{
    float eng = scale(raw);

    // Inicialización
    if (!initialized)
    {
        filtered = eng;
        stable = eng;
        latchedValue = eng;
        initialized = true;
        return;
    }

    // Filtro EMA
    filtered = cfg.alpha * eng + (1.0f - cfg.alpha) * filtered;

    // Histéresis
    if (fabs(filtered - stable) < cfg.hysteresis)
        return;

    stable = filtered;

    if (cfg.latch)
        latchedValue = stable;
}

float AnalogInput::getValue() const
{
    if (cfg.latch)
        return latchedValue;

    return stable;
}

void AnalogInput::resetLatch()
{
    latchedValue = stable;
}
