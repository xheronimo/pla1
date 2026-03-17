#include "web/web_api_i2c_status.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "i2c/i2c_bus.h"
#include "i2c/i2c_metrics.h"
extern SemaphoreHandle_t semI2C;
void registerI2CStatusAPI(AsyncWebServer* server) {
    server->on("/api/i2c/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        
        JsonDocument doc;

        // No es estrictamente necesario el semáforo aquí porque solo leemos contadores 
        // de memoria (atómicos), pero si i2cGetBusHealth() consulta el hardware, añádelo.
        
        doc["ops"]    = i2cGetTotalOps();
        doc["errors"] = i2cGetTotalErrors();
        doc["resets"] = i2cGetTotalResets();
        doc["health"] = i2cGetBusHealth(); // Este valor lo calculamos en i2c_metrics
        doc["uptime"] = millis() / 1000;   // Útil para saber cuánto lleva el bus vivo

        String out;
        serializeJson(doc, out);
        request->send(200, "application/json", out);
    });
}