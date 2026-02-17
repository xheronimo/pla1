#pragma once
#include <stdint.h>

struct SystemHealth
{
    bool modbusOk;
    bool i2cOk;
    bool netOk;
    bool mqttOk;

    uint32_t modbusErrors;
    uint32_t i2cErrors;

    uint32_t uptimeSec;
};

SystemHealth getSystemHealth();

