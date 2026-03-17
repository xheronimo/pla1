#ifndef WEB_API_HEALTH_H
#define WEB_API_HEALTH_H

#include <ESPAsyncWebServer.h>

/** @brief Salud general (Modbus, I2C, Uptime) */
void registerHealthApi(AsyncWebServer* server);

#endif