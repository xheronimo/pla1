#include "web/api_i2c_drivers.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "i2c/i2c_chip_registry.h"
#include "i2c/i2c_bus.h"
extern SemaphoreHandle_t semI2C;
void registerI2CDriversAPI(AsyncWebServer* server) {
    
    server->on("/api/i2c/drivers", HTTP_GET, [](AsyncWebServerRequest *request) {
        
        // Usamos un buffer de 4KB para cubrir todos los drivers posibles
        DynamicJsonDocument doc(4096);
        JsonArray arr = doc.to<JsonArray>();

        size_t count = i2cGetDriverCount();

        for (size_t i = 0; i < count; i++) {
            const I2CChipDriver* drv = i2cGetDriverByIndex(i);
            if (!drv) continue;

            // Extraemos los metadatos del driver (Nombre, canales, etc.)
            ChipMetadata meta;
            drv->meta(meta);

            JsonObject o = arr.createNestedObject();
            o["type"]       = (int)drv->type;
            o["name"]       = meta.name;
            o["channels"]   = meta.channelCount;
            o["autoDetect"] = drv->allowAutoDetect;
            
            // Opcional: Podrías añadir si el driver es de entrada o salida
            // o["kind"]    = (int)meta.kind; 
        }

        String out;
        serializeJson(doc, out);
        
        // Enviamos la respuesta de forma asíncrona
        request->send(200, "application/json", out);
    });
}