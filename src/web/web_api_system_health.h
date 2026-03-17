#ifndef WEB_API_SYSTEM_HEALTH_H
#define WEB_API_SYSTEM_HEALTH_H

#include <ESPAsyncWebServer.h>

/** @brief Salud profunda del sistema (Heap, Motivos de Reset, Watchdogs) */
void registerSystemHealthAPI(AsyncWebServer* server);

#endif