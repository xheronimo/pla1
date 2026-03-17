#ifndef WEB_API_WATCHDOG_HEALTH_H
#define WEB_API_WATCHDOG_HEALTH_H

#include <ESPAsyncWebServer.h>

/** @brief Detalle de los 13 canales de salud de las tareas */
void registerWatchdogHealthAPI(AsyncWebServer* server);

#endif