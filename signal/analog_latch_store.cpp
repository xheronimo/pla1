#include "analog_latch_store.h"
#include <Preferences.h>

static Preferences prefs;

bool loadAnalogLatch(const String& id, float& value)
{
    prefs.begin("a_latches", true);
    if (!prefs.isKey(id.c_str()))
    {
        prefs.end();
        return false;
    }

    value = prefs.getFloat(id.c_str(), 0.0f);
    prefs.end();
    return true;
}

void saveAnalogLatch(const String& id, float value)
{
    prefs.begin("a_latches", false);
    prefs.putFloat(id.c_str(), value);
    prefs.end();
}
