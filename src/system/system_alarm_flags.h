#pragma once
#include <stdint.h>

enum SystemAlarm : uint32_t
{
    ALARM_NONE            = 0,
    ALARM_I2C_STUCK       = (1 << 0),
    ALARM_MODBUS_TIMEOUT  = (1 << 1),
    ALARM_MODBUS_CONFIG   = (1 << 2),
    ALARM_WATCHDOG_RISK   = (1 << 3),
};

void systemAlarmEvaluate();
uint32_t systemAlarmGetFlags();
