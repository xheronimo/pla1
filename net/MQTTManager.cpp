#include "MQTTManager.h"

// ===============================
// STUB (no implementación real)
// ===============================

static bool mqttEnabled = false;





void MQTTManager::configurar(
    const char*,
    uint16_t,
    const char*,
    const char*,
    const char*
)
{
    // stub
}

void MQTTManager::habilitar(bool en)
{
    mqttEnabled = en;
}

bool MQTTManager::conectado()
{
    return mqttEnabled;
}

void MQTTManager::publicarEvento(const AlarmEvent&)
{
    // stub: no hace nada
}

void MQTTManager::init(){}