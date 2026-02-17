#pragma once
#include "modbus_manager.h"
#include <Arduino.h>

void pollSlaveByType(uint8_t slave, ModbusRegType type);
