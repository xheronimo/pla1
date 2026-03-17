#include "web_api_system_reset.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "esp_system.h"

static const char* resetReasonFull(esp_reset_reason_t r) {
    switch (r) {
        case ESP_RST_POWERON:   return "Power On / Hard Reset";
        case ESP_RST_SW:        return "Software Reboot";
        case ESP_RST_PANIC:     return "System Panic (Crash)";
        case ESP_RST_WDT:       return "Watchdog Timer Expiration";
        case ESP_RST_BROWNOUT:  return "Brownout (Voltage Drop)";
        case ESP_RST_DEEPSLEEP: return "Wakeup from Sleep";
        default:                return "Unknown Reason";
    }
}

void registerSystemResetAPI(AsyncWebServer* server) {
    server->on("/api/system/reset_info", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;
        esp_reset_reason_t r = esp_reset_reason();

        doc["reason_code"] = (int)r;
        doc["reason_text"] = resetReasonFull(r);
        doc["uptimeMs"] = millis();

        String out;
        serializeJson(doc, out);
        request->send(200, "application/json", out);
    });
}