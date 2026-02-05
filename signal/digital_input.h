#pragma once
#include "digital_input.h"

class DigitalInput
{
public:
    DigitalInput(const DigitalInputConfig& cfg);

    void update(bool rawValue);   // valor leído del HW
    float getValue() const;       // 0 / 1 para SignalStore
    void resetLatch();

private:
    DigitalInputConfig cfg;

    bool lastRaw = false;
    bool stable = false;
    bool latched = false;

    uint32_t lastChangeMs = 0;
};
#include "digital_latch_store.h"
#include <Preferences.h>

static Preferences prefs;

bool loadLatch(const String& id)
{
    prefs.begin("latches", true);
    bool v = prefs.getBool(id.c_str(), false);
    prefs.end();
    return v;
}

void saveLatch(const String& id, bool value)
{
    prefs.begin("latches", false);
    prefs.putBool(id.c_str(), value);
    prefs.end();
}

void clearAllLatches()
{
    prefs.begin("latches", false);
    prefs.clear();
    prefs.end();
}
