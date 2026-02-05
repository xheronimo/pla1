#include "signal_manager.h"
#include "sensor/sensor_manager.h"

// ⚠️ Tabla global simple (luego vendrá por JSON)
extern Signal signalTable[];
extern uint8_t signalCount;

static Signal* buscarSignal(const char* id)
{
    for (uint8_t i = 0; i < signalCount; i++)
        if (strcmp(signalTable[i].id, id) == 0)
            return &signalTable[i];
    return nullptr;
}

bool getSignalValue(const char* signalId, float& outValue)
{
    Signal* s = buscarSignal(signalId);
    if (!s) return false;

    if (!leerSensor(s->sensor, s->valor))
        return false;

    outValue = s->valor;
    return true;
}

bool getSignalState(const char* signalId, bool& outState)
{
    float v;
    if (!getSignalValue(signalId, v))
        return false;

    outState = (v > 0.5f);
    return true;
}
