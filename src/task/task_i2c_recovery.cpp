#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_chip_registry.h"
#include "i2c/i2c_bus.h"

#include <Arduino.h>

#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_chip_registry.h"
#include "i2c/i2c_bus.h"

#include <Arduino.h>

#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_chip_registry.h"
#include "i2c/i2c_watchdog.h"
#include "i2c/i2c_bus.h"
#include "system/system_alarm_flags.h"

#include <Arduino.h>

static void taskI2CRecovery(void *pvParameters)
{
    (void)pvParameters;

    const TickType_t delay = pdMS_TO_TICKS(2000);

    for (;;)
    {
        uint32_t now = millis();

        for (int t = 0; t < I2C_MAX_TYPES; t++)
        {
            for (int i = 0; i < I2C_MAX_CHIPS_PER_TYPE; i++)
            {
                ChipContext *ctx = i2cGetChipContext((I2CDevice)t, i);
                if (!ctx)
                    continue;

                if (!ctx->inUse)
                    continue;

                if (!ctx->disabled)
                    continue;

                // Cooldown antes de intentar reactivar
                if (now - ctx->lastReadTs < ctx->retryMs)
                    continue;

                // Rehabilitación suave
                ctx->disabled = false;
                ctx->state = ChipState::STATE_UNINITIALIZED;
                ctx->consecutiveErrors = 0;

                // Si hubo muchísimos errores → reset físico bus
                if (ctx->totalErrors > 50)
                {
                    i2cBusReset();
                }
            }
        }
        systemAlarmEvaluate();
        vTaskDelay(delay);
    }
}

void startI2CRecoveryTask()
{
    xTaskCreatePinnedToCore(
        taskI2CRecovery,
        "I2C_RECOVERY",
        4096,
        nullptr,
        1,
        nullptr,
        0
    );
}