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
    // Usamos un tamaño mayor porque ahora enviamos más campos
    JsonDocument doc;
    JsonArray arr = doc["signals"].to<JsonArray>();

    size_t count;
    Signal* table = signalManagerGetAll(count);

    for (size_t i = 0; i < count; i++)
    {
        Signal& s = table[i];
        JsonObject o = arr.add<JsonObject>();

        // Identificación y Nombres
        o["id"]       = s.id;
        o["name"]     = s.name; // ¡Nuevo!
        o["deviceId"] = s.deviceId; // ¡Nuevo!

        // Configuración de Hardware
        o["bus"]     = (int)s.bus;
        o["kind"]    = (int)s.kind;
        o["address"] = s.address; // Útil para la UI
        o["channel"] = s.channel;

        // Estado en tiempo real
        o["value"]   = s.value;
        o["error"]   = s.error;

        // Flags de interfaz (Para que el frontend bloquee botones)
        o["system"]  = s.systemReserved; 
        o["locked"]  = s.lockedConfig;
        o["writable"] = s.writable;

        // Datos de calibración extendidos
        if (s.kind == SignalKind::SENSOR_ANALOG)
        {
            JsonObject c = o["calib"].to<JsonObject>();
            c["rawMin"]  = s.calib.rawMin;
            c["rawMax"]  = s.calib.rawMax;
            c["realMin"] = s.calib.realMin;
            c["realMax"] = s.calib.realMax;
            c["clamp"]   = s.calib.clamp;
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
