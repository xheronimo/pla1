#include "digital_input.h"

DigitalInput::DigitalInput(const DigitalInputConfig& c)
    : cfg(c)
{
}

void DigitalInput::update(bool raw)
{
    if (cfg.invertido)
        raw = !raw;

    uint32_t now = millis();

    if (raw != lastRaw)
    {
        lastRaw = raw;
        lastChangeMs = now;
        return;
    }

    if ((now - lastChangeMs) < cfg.debounceMs)
        return;

    // estado estable
    stable = raw;

    if (cfg.latch && stable)
        latched = true;
}

float DigitalInput::getValue() const
{
    if (cfg.latch)
        return latched ? 1.0f : 0.0f;

    return stable ? 1.0f : 0.0f;
}

void DigitalInput::resetLatch()
{
    latched = false;
}
