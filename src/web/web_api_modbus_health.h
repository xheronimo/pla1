#ifndef WEB_API_MODBUS_HEALTH_H
#define WEB_API_MODBUS_HEALTH_H

#include <ESPAsyncWebServer.h>

/** @brief Estado de comunicación con esclavos Modbus */
void registerModbusHealthApi(AsyncWebServer* server);

#endif