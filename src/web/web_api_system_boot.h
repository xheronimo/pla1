#ifndef WEB_API_SYSTEM_BOOT_H
#define WEB_API_SYSTEM_BOOT_H

#include <ESPAsyncWebServer.h>

/** @brief Registra la ruta GET /api/system/boot */
void registerSystemBootApi(AsyncWebServer* server);

#endif