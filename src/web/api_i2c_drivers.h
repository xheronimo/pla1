#ifndef API_I2C_DRIVERS_H
#define API_I2C_DRIVERS_H

#include <ESPAsyncWebServer.h>

/**
 * @brief Registra el endpoint para listar los drivers I2C soportados.
 * GET /api/i2c/drivers
 */
void registerI2CDriversAPI(AsyncWebServer* server);

#endif