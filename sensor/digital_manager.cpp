#include "digital_manager.h"

std::vector<DigitalSensor> DigitalManager::sensores;

void DigitalManager::init(const std::vector<DigitalSensorConfig>& cfgs)
{
    sensores.clear();
    for (auto& c : cfgs)
        sensores.emplace_back(c);
}

void DigitalManager::loop()
{
    for (auto& s : sensores)
        s.update();
}

DigitalSensor* DigitalManager::getById(const String& id)
{
    for (auto& s : sensores)
        if (s.getId() == id)
            return &s;
    return nullptr;
}
