#include "modbus/modbus_scheduler.h"
#include "system/WatchdogManager.h"
#include "modbus/modbus_config.h"
#include "system/system_event_logger.h"
#include "system/system_alarm_flags.h"

#include "modbus/modbus_manager.h"
#include "system/LogSystem.h"
#include "system/system_safe_mode.h"
#include <Arduino.h>


static void taskModbusPoll(void* pv)
{
    (void)pv;

    watchdogRegister(WDT_MODBUS);

    const TickType_t loopDelay = pdMS_TO_TICKS(20); // loop rápido
    uint32_t lastPollMs = 0;

    for (;;)
    {
        uint32_t now = millis();

        // Ejecutar scheduler solo cuando toca
        if (now - lastPollMs >= MODBUS_DEFAULT_POLL_MS)
        {
            modbusPollScheduler();
            lastPollMs = now;
        }
        systemAlarmEvaluate();
        modbusCheckRecovery();
        logSystemAlarm(systemAlarmGetFlags());
        // Alimentar watchdog
        watchdogKick(WDT_MODBUS);

        vTaskDelay(loopDelay);
    }
}

void startModbusTask()
{
    if (systemInSafeMode())
        return;

    ModbusManager::init(Serial2, 9600);

    xTaskCreatePinnedToCore(
        taskModbusPoll,
        "MODBUS",
        4096,
        nullptr,
        2,
        nullptr,
        1
    );
}