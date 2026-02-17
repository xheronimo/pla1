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

Signal* signalRegistryGetAll(size_t& count)
{
    return signalManagerGetAll(count);
}

size_t signalRegistryGetCount()
{
    return signalManagerGetCount();
}