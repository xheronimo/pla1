#include "web/api_i2c_create.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "signal/signal_factory.h"
#include "i2c/i2c_bus.h" 
#include "system/LogSystem.h"
#include "storage/SDManager.h" // Para logging seguro

// Semáforo global del bus I2C
extern SemaphoreHandle_t semI2C;

void registerI2CCreateAPI(AsyncWebServer* server) {
    
    // El handler de Body es necesario para capturar el JSON del POST
    server->on("/api/i2c/create", HTTP_POST, [](AsyncWebServerRequest *request) {
        // El handler principal se deja vacío si usamos el body handler
    }, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        
        // 1. ArduinoJson 7: No necesita tamaño fijo, gestiona el heap eficientemente
        JsonDocument doc; 
        DeserializationError error = deserializeJson(doc, data, len);

        if (error) {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON body\"}");
            return;
        }

        // 2. Extraer parámetros con valores por defecto (Seguridad)
        I2CDevice type = (I2CDevice)(doc["type"] | 0);
        uint8_t addr   = doc["address"] | 0x00;
        bool reserved  = doc["reserved"] | false;

        const char* chipName = i2cGetDriverName(type);
        if (!chipName) {
            request->send(400, "application/json", "{\"error\":\"Unknown I2C type\"}");
            return;
        }

        // 3. Crear señales protegiendo el acceso al Registry/I2C
        // Aunque aquí no leemos el chip, la factory podría inicializar el hardware
        int createdCount = 0;
        JsonArray channels = doc["channels"].as<JsonArray>();

        if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(200)) == pdTRUE) {
            
            for (JsonVariant v : channels) {
                uint8_t ch = v.as<uint8_t>();

                char id[32];
                char name[32];

                // Formateo seguro sin usar Strings dinámicos
                snprintf(id, sizeof(id), "%s_%02X_CH%u", chipName, addr, ch);
                snprintf(name, sizeof(name), "I2C %s %u", chipName, ch);

                // Inyectar en el sistema
                addSystemSignal(
                    id, name, type, addr, ch,
                    BusType::BUS_I2C,
                    SignalKind::SENSOR_ANALOG, 
                    900 + addr,
                    reserved, 
                    reserved 
                );
                createdCount++;
            }
            xSemaphoreGive(semI2C);
        } else {
            request->send(503, "application/json", "{\"error\":\"I2C_BUSY_DURING_CONFIG\"}");
            return;
        }

        // 4. Log y Respuesta final
        char logBuf[64];
        snprintf(logBuf, sizeof(logBuf), "WEB: Creadas %d señales (Chip 0x%02X)", createdCount, addr);
        SDMgr::logEvent("INFO", logBuf);

        // Respuesta optimizada: Buffer estático para evitar fragmentación
        char response[64];
        snprintf(response, sizeof(response), "{\"status\":\"ok\",\"created\":%d}", createdCount);
        request->send(200, "application/json", response);
    });
}