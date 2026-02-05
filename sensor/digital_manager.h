#pragma once
#include <vector>
#include "digital_sensor.h"

class DigitalManager
{
public:
    static void init(const std::vector<DigitalSensorConfig>& cfgs);
    static void loop();

    static DigitalSensor* getById(const String& id);

private:
    static std::vector<DigitalSensor> sensores;
};
