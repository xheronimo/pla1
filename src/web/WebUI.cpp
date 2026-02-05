#include "WebUI.h"

#include <WebServer.h>
#include <ArduinoJson.h>

#include "alarm/alarm_runtime.h"
#include "alarm/alarm_ack.h"
#include "system/LogSystem.h"

static WebServer server(80);

// =====================================================

static void handleStatus()
{
    StaticJsonDocument<512> doc;
    JsonArray arr = doc.createNestedArray("alarms");

    for (auto& r : alarmRuntimeAll())
    {
        JsonObject o = arr.createNestedObject();
        o["id"]     = r.id;
        o["active"] = r.activaAhora;
        o["latched"]= r.latched;
        o["acked"]  = alarmIsAcked(r.id);
    }

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

// =====================================================

static void handleAck()
{
    if (!server.hasArg("id"))
    {
        server.send(400, "text/plain", "missing id");
        return;
    }

    alarmAck(server.arg("id").c_str());
    server.send(200, "text/plain", "OK");
}

// =====================================================

void webInit()
{
    server.on("/api/status", handleStatus);
    server.on("/api/ack", handleAck);
    server.begin();

    escribirLog("WEB:READY");
}

void webLoop()
{
    server.handleClient();
}
