#include "system_alarm_flags.h"
#include "i2c/i2c_bus.h"
#include "modbus/modbus_slave_context.h"
#include "system/WatchdogManager.h"
#include "i2c/i2c_watchdog.h"

static uint32_t g_activeAlarms = 0;

void systemAlarmEvaluate()
{
    g_activeAlarms = 0;

    // ---- I2C ----
    if (i2cBusIsStuck())
        g_activeAlarms |= ALARM_I2C_STUCK;

    // ---- Modbus ----
    for (uint8_t i = 1; i <= MODBUS_MAX_SLAVES; i++)
    {
        ModbusSlaveContext* ctx = modbusGetContext(i);
        if (!ctx || !ctx->inUse)
            continue;

        if (ctx->state == ModbusSlaveState::STATE_TIMEOUT ||
            ctx->state == ModbusSlaveState::STATE_DISABLED)
            g_activeAlarms |= ALARM_MODBUS_TIMEOUT;

        if (ctx->configErrors > 0)
            g_activeAlarms |= ALARM_MODBUS_CONFIG;
    }
}

uint32_t systemAlarmGetFlags()
{
    return g_activeAlarms;
}
