#ifndef API_I2C_SCAN_H
#define API_I2C_SCAN_H

#include <ESPAsyncWebServer.h>

/**
 * @brief Registra el endpoint para realizar un escaneo automático del bus I2C.
 * GET /api/i2c/scan
 */
void registerI2CScanAPI(AsyncWebServer* server);

#endif