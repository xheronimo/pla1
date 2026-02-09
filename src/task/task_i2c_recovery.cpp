#include "task_i2c_recovery.h"

void taskI2CRecovery(void* pvParameters)
{
    (void) pvParameters;

    const TickType_t delay = pdMS_TO_TICKS(1000);

    for (;;)
    {
        uint32_t now = millis();

        for (size_t i = 0; i < signalCount; i++)
        {
            const Signal& s = signalTable[i];

            if (s.bus != BusType::BUS_I2C)
                continue;

            ChipContext* cc = i2cGetChipContext(
                s.bus,
                s.chip,
                s.address
            );

            if (!cc)
                continue;

            // 🔴 SOLO chips en ERROR
            if (cc->state != ChipState::ERROR)
                continue;

            // ⏱ Esperar retryMs
            if (now - cc->lastReadTs < cc->retryMs)
                continue;

            // 🔄 Reintentar
            cc->state = ChipState::UNINITIALIZED;
            cc->consecutiveErrors = 0;
        }

        vTaskDelay(delay);
    }
}
