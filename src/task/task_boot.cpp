#include "task_boot.h"
#include "system/LogSystem.h"

#include "task_watchdog.h"
#include "task_web.h"
#include "task_display.h"
#include "task_i2c_recovery.h"
#include "modbus/modbus_poll_task.h"
#include "task_signal_polling.h"
#include "task_alarm_rules.h"
#include "task_alarm_dispatcher.h"
#include "system/WatchdogManager.h"
#include "system/system_safe_mode.h"

void arrancarTareasMinimas()
{
    watchdogInit(30000);

    startWebTask();
    startI2CRecoveryTask();
    startDisplayTask();   // internamente mira cfg.enableDisplay
}

void arrancarTareasNormales()
{
     if (systemInSafeMode())
        return;
    startSignalPollingTask();
    startAlarmRulesTask();
    startAlarmDispatcherTask();
    startModbusTask();
}
