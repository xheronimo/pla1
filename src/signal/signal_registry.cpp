#include "signal_registry.h"
#include "signal_manager.h"

void signalRegistryInit()
{
    signalManagerInit();
}

void signalRegistryPoll()
{
    signalManagerPollAll();
}

bool signalRegistryGet(const char* id, float& outValue)
{
    return signalManagerGetValue(id, outValue);
}
