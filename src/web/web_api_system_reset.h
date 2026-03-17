#ifndef WEB_API_SYSTEM_RESET_H
#define WEB_API_SYSTEM_RESET_H

#include <ESPAsyncWebServer.h>

/** @brief Comando para reiniciar el PLC y contador WDT */
void registerSystemResetAPI(AsyncWebServer* server);

#endif