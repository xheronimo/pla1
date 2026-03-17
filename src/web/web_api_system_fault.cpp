#include "web/web_api_system_fault.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "system/fault_output.h"
#include "system/system_fault.h"
#include "signal/signal_manager.h"

extern bool g_lastResetWdt;
extern bool g_faultAcked;

void registerSystemFaultApi(AsyncWebServer* server) {
    // Manejamos GET y POST en el mismo endpoint de forma asíncrona
    server->on("/api/system/fault", HTTP_ANY, [](AsyncWebServerRequest *request) {
        
        if (request->method() == HTTP_GET) {
            JsonDocument doc;
            doc["fault"]    = systemHasCriticalFault();
            doc["resetWdt"] = g_lastResetWdt;
            doc["acked"]    = g_faultAcked;

            Signal* s = SignalMgr::getById("FAULT_WDT");
            doc["output"] = s ? (s->value > 0.5f) : false;

            String out;
            serializeJson(doc, out);
            request->send(200, "application/json", out);
        } 
        else if (request->method() == HTTP_POST) {
            systemAcknowledgeFault();
            faultOutputClear();
            request->send(200, "application/json", "{\"acknowledged\":true}");
        }
    });
}