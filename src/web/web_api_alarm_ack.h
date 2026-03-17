#ifndef WEB_API_ALARM_ACK_H
#define WEB_API_ALARM_ACK_H

#include <ESPAsyncWebServer.h>

/** @brief Registra la ruta POST /api/alarm/ack */
void registerAlarmAckApi(AsyncWebServer* server);

#endif