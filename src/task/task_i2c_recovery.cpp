#include "tasks/task_i2c_recovery.h"
#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_chip_registry.h"
#include "i2c/i2c_bus.h"
#include "i2c/i2c_watchdog.h"
#include "system/system_alarm_flags.h"
#include "system/WatchdogManager.h" // <--- Inyectado
#include "system/LogSystem.h"

static void taskI2CRecovery(void *pvParameters)
{
    (void)pvParameters;
    // Registro de la tarea en el supervisor
    watchdogRegister(WDT_I2C);

    const TickType_t delay = pdMS_TO_TICKS(2000);

    for (;;)
    {
        // Kick al supervisor: "El recuperador de bus sigue vivo"
        watchdogKick(WDT_I2C);

        uint32_t now = millis();

        for (int t = 0; t < I2C_MAX_TYPES; t++)
        {
            for (int i = 0; i < I2C_MAX_CHIPS_PER_TYPE; i++)
            {
                ChipContext *ctx = i2cGetChipContext((I2CDevice)t, i);
                if (!ctx || !ctx->inUse || !ctx->disabled)
                    continue;

                // Cooldown antes de intentar reactivar
                if (now - ctx->lastReadTs < ctx->retryMs)
                    continue;

                // Rehabilitación suave
                ctx->disabled = false;
                ctx->state = ChipState::STATE_UNINITIALIZED;
                ctx->consecutiveErrors = 0;

                escribirLog("I2C: Reintentando chip ID %d tipo %d", i, t);

                // Si hubo muchos errores acumulados → reset físico del bus (SCL pulses)
                if (ctx->totalErrors > 50)
                {
                    escribirLog("I2C: Umbral critico superado. Forzando i2cBusReset()");
                    i2cBusReset();
                    ctx->totalErrors = 0; // Resetear contador tras reset físico
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
        2,    // Prioridad 2: Superior a la web
        nullptr,
        0     // Core 0: Para no interferir con el control en Core 1
    );
}