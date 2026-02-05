
#include "signal_store.h"
#include <vector>

struct SignalEntry
{
    String id;
    float value;
};

static std::vector<SignalEntry> signals;
static SemaphoreHandle_t signalMutex = nullptr;

// --------------------------------------------------

static void lock()
{
    if (!signalMutex)
        signalMutex = xSemaphoreCreateMutex();

    xSemaphoreTake(signalMutex, portMAX_DELAY);
}

static void unlock()
{
    xSemaphoreGive(signalMutex);
}

// --------------------------------------------------

bool getSignalValue(const String& id, float& outValue)
{
    lock();

    for (auto& s : signals)
    {
        if (s.id == id)
        {
            outValue = s.value;
            unlock();
            return true;
        }
    }

    unlock();
    return false;
}

// --------------------------------------------------

void setSignalValue(const String& id, float value)
{
    lock();

    for (auto& s : signals)
    {
        if (s.id == id)
        {
            s.value = value;
            unlock();
            return;
        }
    }

    signals.push_back({ id, value });
    unlock();
}

// --------------------------------------------------

void clearSignal(const String& id)
{
    lock();

    for (auto it = signals.begin(); it != signals.end(); ++it)
    {
        if (it->id == id)
        {
            signals.erase(it);
            break;
        }
    }

    unlock();
}

// --------------------------------------------------

void clearAllSignals()
{
    lock();
    signals.clear();
    unlock();
}
