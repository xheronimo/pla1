#include "api_signals.h"

#include <WebServer.h>
#include <ArduinoJson.h>

#include "signal/signal_manager.h"
#include "signal/signal_persist.h"
#include "system/LogSystem.h"
#include "signal/signal_persist.h"


extern WebServer server;

// =======================================================
// GET /api/signals
// =======================================================
static void handleGetSignals()
{
    JsonDocument doc;
    JsonArray arr = doc["signals"].to<JsonArray>();

    size_t count;
    Signal* table = signalManagerGetAll(count);

    for (size_t i = 0; i < count; i++)
    {
        Signal& s = table[i];
        JsonObject o = arr.add<JsonObject>();

        o["id"]      = s.id;
        o["bus"]     = (int)s.bus;
        o["kind"]    = (int)s.kind;
        o["value"]   = s.value;
        o["raw"]     = s.raw;
        o["quality"] = (int)s.quality;
        o["valid"]   = s.valid;
        o["error"]   = s.error;

        if (s.kind == SignalKind::SENSOR_ANALOG)
        {
            JsonObject c = o["calib"].to<JsonObject>();
            c["offset"]     = s.calib.offset;
            c["hysteresis"] = s.calib.measureHysteresis;
            c["ema"]        = s.calib.emaAlpha;
        }
    }

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

// =======================================================
// PUT /api/signals/{id}
// Body:
// {
//   "offset": 1.2,
//   "hysteresis": 0.5,
//   "ema": 0.2
// }
// =======================================================
static void handlePutSignal()
{
    if (!server.hasArg("plain"))
    {
        server.send(400, "application/json", "{\"error\":\"no body\"}");
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, server.arg("plain")) != DeserializationError::Ok)
    {
        server.send(400, "application/json", "{\"error\":\"bad json\"}");
        return;
    }

    // Obtener ID desde la URL
    String uri = server.uri();              // /api/signals/ID
    String id  = uri.substring(uri.lastIndexOf('/') + 1);

    Signal* s = signalManagerGetById(id.c_str());
    if (!s)
    {
        server.send(404, "application/json", "{\"error\":\"signal not found\"}");
        return;
    }

    if (s->kind != SignalKind::SENSOR_ANALOG)
    {
        server.send(400, "application/json", "{\"error\":\"not analog\"}");
        return;
    }

    bool changed = false;

    if (doc.containsKey("offset"))
    {
        s->calib.offset = doc["offset"].as<float>();
        changed = true;
    }

    if (doc.containsKey("hysteresis"))
    {
        s->calib.measureHysteresis = doc["hysteresis"].as<float>();
        changed = true;
    }

    if (doc.containsKey("ema"))
    {
        s->calib.emaAlpha = doc["ema"].as<float>();
        changed = true;
    }

    if (changed)
    {
        // 🔁 Forzar recalculo limpio
        s->initialized = false;
        s->calib.hasStableValue = false;
        s->calib.emaInit = false;

        // 💾 PERSISTENCIA AQUÍ
        saveSignalCalibration(*s);

        escribirLog("SIGNAL: Calibracion actualizada (%s)", s->id);
    }

    server.send(200, "application/json", "{\"ok\":true}");
}

// =======================================================
// Registro
// =======================================================
void registerSignalApi()
{
    server.on("/api/signals", HTTP_GET, handleGetSignals);

    // Ruta dinámica
    server.onNotFound([]()
    {
        if (server.method() == HTTP_PUT &&
            server.uri().startsWith("/api/signals/"))
        {
            handlePutSignal();
            return;
        }

        server.send(404, "application/json", "{\"error\":\"not found\"}");
    });
}
