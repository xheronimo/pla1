#include "signal_registry.h"
#include "signal_sources.h"
#include "signal_struct.h"

// ---- Lista global (vendrá del JSON) ----
static SignalDef* signals = nullptr;
static size_t numSignals = 0;

void signalRegistryInit()
{
    // vacío de momento
}

bool signalGetValue(const String& id, float& outValue)
{
    for (size_t i = 0; i < numSignals; i++)
    {
        if (id == signals[i].id)
        {
            if (!resolveSignal(signals[i]))
                return false;

            outValue = signals[i].value;
            return true;
        }
    }
    return false;
}
