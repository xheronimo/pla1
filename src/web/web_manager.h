#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <ESPAsyncWebServer.h>

/**
 * @namespace WebMgr
 * @brief Orquestador del servidor web asíncrono.
 * * Se encarga de centralizar el registro de todas las APIs (I2C, Modbus, 
 * Alarmas, Señales) y de servir los archivos estáticos desde la SD.
 */
namespace WebMgr {

    /**
     * @brief Inicializa el servidor y registra todas las rutas de la API.
     * @param srv Puntero a la instancia del servidor (normalmente declarada en el main).
     */
    void init(AsyncWebServer* srv);

} // namespace WebMgr

#endif // WEB_MANAGER_H