#include "web_api_system_fault.h"
#include <WebServer.h>
#include <ArduinoJson.h>

#include "system/fault_output.h"
#include "system/system_health.h"
#include "signal/signal_manager.h"
#include "system/system_fault.h"
#include "system/boot_reason.h"



extern WebServer server;
extern bool g_lastResetWdt;
extern bool g_faultAcked;

static void handleApiSystemFault()
{
    // =========================
    // GET → estado del sistema
    // =========================
    if (server.method() == HTTP_GET)
    {
        JsonDocument doc;

        doc["fault"]    = systemHasCriticalFault();
        doc["resetWdt"] = g_lastResetWdt;
        doc["acked"]    = g_faultAcked;

        // Estado físico del relé (solo informativo)
        Signal* s = signalManagerGetById("FAULT_WDT");
        doc["output"] = s ? (s->value > 0.5f) : false;

        String out;
        serializeJson(doc, out);
        server.send(200, "application/json", out);
    }

    // =========================
    // POST → ACK del fallo
    // =========================
    else if (server.method() == HTTP_POST)
    {
        systemAcknowledgeFault();
        faultOutputClear();

        server.send(
            200,
            "application/json",
            "{\"acknowledged\":true}"
        );
    }
}

void registerSystemFaultApi()
{
    server.on("/api/system/fault", HTTP_GET,  handleApiSystemFault);
    server.on("/api/system/fault", HTTP_POST, handleApiSystemFault);
}



