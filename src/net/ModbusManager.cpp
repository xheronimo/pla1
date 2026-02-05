#include "ModbusManager.h"
#include <ModbusRTU.h>

static ModbusRTU mb;

bool ModbusManager::init()
{
    Serial2.begin(9600, SERIAL_8N1);
    mb.begin(&Serial2);
    mb.master();
    return true;
}

bool ModbusManager::readHolding(uint8_t slave, uint16_t address, float& out)
{
    uint16_t reg;
    if (!mb.readHreg(slave, address, &reg, 1))
        return false;

    while (mb.slave()) mb.task();

    out = (float)reg;
    return true;
}

bool ModbusManager::readDiscrete(uint8_t slave, uint16_t address, bool& out)
{
    uint16_t reg;
    if (!mb.readIsts(slave, address, &reg, 1))
        return false;

    while (mb.slave()) mb.task();

    out = reg != 0;
    return true;
}
