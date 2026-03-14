#include "system_event_logger.h"
#include "system/LogSystem.h"
#include "system_alarm_flags.h"
#include <stdint.h>

void logSystemAlarm(uint32_t flags)
{
    if (flags & ALARM_I2C_STUCK)
        escribirLog("ALARM:I2C:STUCK");

    if (flags & ALARM_MODBUS_TIMEOUT)
        escribirLog("ALARM:MODBUS:TIMEOUT");

    if (flags & ALARM_MODBUS_CONFIG)
        escribirLog("ALARM:MODBUS:CONFIG");

    if (flags & ALARM_WATCHDOG_RISK)
        escribirLog("ALARM:WATCHDOG:RISK");
}
