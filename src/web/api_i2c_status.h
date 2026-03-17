#ifndef WEB_API_I2C_STATUS_H
#define WEB_API_I2C_STATUS_H

#include <ESPAsyncWebServer.h>

/**
 * @brief Registra el endpoint para el estado global del bus I2C.
 * GET /api/i2c/status
 */
void registerI2CStatusAPI(AsyncWebServer* server);

#endif