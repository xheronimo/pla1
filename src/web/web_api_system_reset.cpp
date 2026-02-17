#include "web_api_system_reset.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include "esp_system.h"


extern WebServer server;

static const char* resetReasonToStr(esp_reset_reason_t r)
{
    switch (r)
    {
        case ESP_RST_POWERON: return "power_on";
        case ESP_RST_SW:      return "software";
        case ESP_RST_PANIC:   return "panic";
        case ESP_RST_WDT:     return "watchdog";
        case ESP_RST_BROWNOUT:return "brownout";
        case ESP_RST_DEEPSLEEP:return "deep_sleep";
        default:              return "unknown";
    }
}

static void handleApiSystemReset()
{
    JsonDocument doc;

    esp_reset_reason_t r = esp_reset_reason();

    doc["reason"] = resetReasonToStr(r);
    doc["uptimeMs"] = millis();

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void registerSystemResetAPI()
{
    server.on("/api/system/reset", HTTP_GET, handleApiSystemReset);
}
