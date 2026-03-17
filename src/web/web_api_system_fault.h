#ifndef WEB_API_SYSTEM_FAULT_H
#define WEB_API_SYSTEM_FAULT_H

#include <ESPAsyncWebServer.h>

/** @brief Registra las rutas GET y POST para /api/system/fault */
void registerSystemFaultApi(AsyncWebServer* server);

#endif