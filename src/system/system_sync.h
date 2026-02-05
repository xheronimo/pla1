#pragma once
#include "freertos/semphr.h"

extern SemaphoreHandle_t semI2C;
extern SemaphoreHandle_t semFS;
extern SemaphoreHandle_t semAlarm;
extern SemaphoreHandle_t semConfig;
extern SemaphoreHandle_t semQueue;
extern SemaphoreHandle_t semAlarmRouter;
extern SemaphoreHandle_t semSD;

void systemSyncInit();