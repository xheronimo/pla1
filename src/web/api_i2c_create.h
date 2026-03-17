#ifndef API_I2C_CREATE_H
#define API_I2C_CREATE_H

#include <ESPAsyncWebServer.h>

/** * @brief Registra el endpoint para crear señales I2C dinámicamente.
 * POST /api/i2c/create
 */
void registerI2CCreateAPI(AsyncWebServer* server);

#endif