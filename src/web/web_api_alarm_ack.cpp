#include "web_api_alarm_ack.h"
#include "alarm/alarm_manager.h"
#include "system/LogSystem.h"
#include <ESPAsyncWebServer.h>

/**
 * @brief Registra el endpoint para el Acknowledge de alarmas.
 * Unifica la lógica de alarm_runtime y alarm_manager.
 */
void registerAlarmAckApi(AsyncWebServer* server) {
    
    // POST /api/alarm/ack
    server->on("/api/alarm/ack", HTTP_POST, [](AsyncWebServerRequest *request) {
        
        uint32_t alarmId = 0;
        bool found = false;

        // 1. Buscamos el ID en el cuerpo del POST (true)
        if (request->hasParam("id", true)) {
            alarmId = request->getParam("id", true)->value().toInt();
            found = true;
        } 
        // 2. Fallback: Buscamos el ID en la URL (false)
        else if (request->hasParam("id")) {
            alarmId = request->getParam("id")->value().toInt();
            found = true;
        }

        if (found) {
            // Ejecutamos el ACK en el Manager (esto limpia latch e interlocks)
            AlarmMgr::acknowledge(alarmId);
            
            escribirLog("WEB: ACK ejecutado para Alarma ID %u", alarmId);
            request->send(200, "application/json", "{\"status\":\"ACK_OK\",\"id\":" + String(alarmId) + "}");
        } 
        else {
            request->send(400, "application/json", "{\"error\":\"Missing ID\"}");
        }
    });
}