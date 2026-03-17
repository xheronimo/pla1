#include "web/api_i2c_scan.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "i2c/i2c_autodetect.h"
#include "system/LogSystem.h"
extern SemaphoreHandle_t semI2C;
void registerI2CScanAPI(AsyncWebServer* server) {
    server->on("/api/i2c/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
        
        // 🔒 PROTECCIÓN: Intentamos tomar el bus I2C
        // Le damos un timeout de 500ms. Si el PLC está muy ocupado, mejor no escanear.
        if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(500)) == pdTRUE) {
            
            auto list = i2cAutoDetect();
            
            // Liberamos el bus inmediatamente después de la detección física
            xSemaphoreGive(semI2C);

            JsonDocument doc; // ArduinoJson 7
            JsonArray arr = doc.to<JsonArray>();

            for (auto& c : list) {
                JsonObject o = arr.add<JsonObject>();
                o["type"] = (int)c.type;
                o["address"] = c.address;
                o["name"] = i2cGetDriverName(c.type); // Ahora que lo tenemos, lo usamos
            }

            String out;
            serializeJson(doc, out);
            request->send(200, "application/json", out);
            
            if (list.size() > 0) {
                SDMgr::logEvent("INFO", "WEB: Escaneo I2C completado con éxito.");
            }
        } else {
            // Si no pudimos tomar el semáforo, el bus está bloqueado por el control crítico
            request->send(503, "application/json", "{\"error\":\"I2C_BUS_BUSY\"}");
        }
    });
}