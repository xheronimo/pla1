#ifndef WEB_API_SIGNALS_H
#define WEB_API_SIGNALS_H

#include <ESPAsyncWebServer.h>

/** @brief Registra las rutas GET /api/signals y PUT /api/signals/{id} */
void registerSignalApi(AsyncWebServer* server);

#endif