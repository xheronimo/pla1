#ifndef WEB_API_ALARMS_H
#define WEB_API_ALARMS_H

#include <ESPAsyncWebServer.h>

/** @brief Registra la ruta GET /api/alarms */
void registerAlarmsApi(AsyncWebServer* server);

#endif