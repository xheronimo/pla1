#ifndef WEB_API_I2C_HEALTH_H
#define WEB_API_I2C_HEALTH_H

#include <ESPAsyncWebServer.h>

/** @brief Estado de los chips I2C (PCF8574, etc.) */
void registerI2CHealthAPI(AsyncWebServer* server);

#endif