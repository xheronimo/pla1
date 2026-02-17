#include "task_modbus_watchdog.h"
#include "modbus/modbus_slave_context.h"
#include "modbus/modbus_config.h"
#include <Arduino.h>

void taskModbusWatchdog(void* pv)
{
    const TickType_t delay = pdMS_TO_TICKS(5000);

    for (;;)
    {
        uint32_t now = millis();

        for (int i = 0; i < MODBUS_MAX_SLAVES; i++)
        {
            ModbusSlaveContext* ctx = modbusGetContext(i + 1);
            if (!ctx) continue;

            if (ctx->state == ModbusSlaveState::STATE_READY)
            {
                if (now - ctx->lastOkTs > 30000)
                {
                    ctx->state = ModbusSlaveState::STATE_TIMEOUT;
                }
            }
        }

        vTaskDelay(delay);
    }
}
