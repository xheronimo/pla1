#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_chip_registry.h"
#include "i2c/i2c_bus.h"

#include <Arduino.h>

void taskI2CRecovery(void* pvParameters)
{
    (void) pvParameters;

    const TickType_t delay = pdMS_TO_TICKS(2000); // cada 2s

    for (;;)
    {
        uint32_t now = millis();

        // ---------------------------------------------------
        // Recorremos todos los tipos de chip
        // ---------------------------------------------------
        for (int t = 0; t < I2C_MAX_TYPES; t++)
        {
            for (int i = 0; i < I2C_MAX_CHIPS_PER_TYPE; i++)
            {
                ChipContext* ctx = i2cGetChipContext((I2CDevice)t, i);

                if (!ctx)
                    continue;

                if (ctx->state != ChipState::ERROR)
                    continue;

                // Esperar tiempo de reintento
                if (now - ctx->initTs < ctx->retryMs)
                    continue;

                // ---------------------------------------------------
                // Intentar reset
                // ---------------------------------------------------
                ctx->state = ChipState::UNINITIALIZED;
                ctx->errorCount = 0;
                ctx->consecutiveErrors = 0;
                ctx->fatal = false;

                // Opcional: reset físico del bus si hay muchos errores
                if (ctx->errorCount > 10)
                {
                    i2cBusReset();
                }
            }
        }

        vTaskDelay(delay);
    }
}
