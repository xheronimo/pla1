#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// ==================================================
// SEMÁFOROS GLOBALES
// ==================================================

extern SemaphoreHandle_t semI2C;
extern SemaphoreHandle_t semFS;
extern SemaphoreHandle_t semAlarm;
extern SemaphoreHandle_t semConfig;
extern SemaphoreHandle_t semQueue;
extern SemaphoreHandle_t semAlarmRouter;
extern SemaphoreHandle_t semSD;



// ==================================================
// INIT
// ==================================================

void systemSyncInit();
