#include "net/MQTTManager.h"

void MQTTManager::init()
{
    // Placeholder
}

bool MQTTManager::conectado()
{
    return false;   // por ahora
}

void MQTTManager::publicarAlarma(
    const char* id,
    bool estado,
    const char* mensaje,
    float valor
)
{
    // De momento no hace nada
    // Aquí luego irá PubSubClient / AsyncMqtt
}
