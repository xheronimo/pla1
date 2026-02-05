#include "system_sync.h"

// ==================================================
// SEMÁFOROS
// ==================================================

SemaphoreHandle_t semI2C    = nullptr;
SemaphoreHandle_t semFS     = nullptr;
SemaphoreHandle_t semAlarm  = nullptr;
SemaphoreHandle_t semConfig = nullptr;
SemaphoreHandle_t semQueue = nullptr;
SemaphoreHandle_t semAlarmRouter = nullptr;
SemaphoreHandle_t semSD = nullptr;


// ==================================================
// INIT
// ==================================================

void systemSyncInit()
{
    if (!semI2C)    semI2C              = xSemaphoreCreateMutex();
    if (!semFS)     semFS               = xSemaphoreCreateMutex();
    if (!semAlarm)  semAlarm            = xSemaphoreCreateMutex();
    if (!semConfig) semConfig           = xSemaphoreCreateMutex();
    if (!semQueue)  semQueue            = xSemaphoreCreateMutex();
    if (!semAlarmRouter) semAlarmRouter  = xSemaphoreCreateMutex();
    if (!semSD)      semSD               = xSemaphoreCreateMutex();
}
