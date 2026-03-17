#include "web/web_api_signals.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "signal/signal_manager.h"
#include "signal/signal_persist.h"
#include "system/LogSystem.h"

void registerSignalApi(AsyncWebServer* server) {
    
    // =======================================================
    // GET /api/signals -> Lista completa de señales
    // =======================================================
    server->on("/api/signals", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Usamos DynamicJsonDocument para manejar listas largas de señales
        DynamicJsonDocument doc(16384); 
        JsonArray arr = doc.createNestedArray("signals");

        // Obtenemos las señales del Manager que consolidamos
        auto& signals = SignalMgr::getAll();

        for (auto &s : signals) {
            JsonObject o = arr.createNestedObject();

            // Identificación
            o["id"]       = s.id;
            o["name"]     = s.name;
            o["chip"]     = s.chipName;

            // Configuración
            o["kind"]     = (int)s.kind;
            o["channel"]  = s.channelIdx;
            o["writable"] = s.writable;

            // Estado en tiempo real
            o["value"]    = serialized(String(s.value, 2)); // 2 decimales
            o["raw"]      = serialized(String(s.raw, 2));
            o["q"]        = (int)s.quality;
            o["valid"]    = s.valid;

            // Datos de calibración
            JsonObject c = o.createNestedObject("calib");
            c["offset"]   = s.calib.offset;
            c["alpha"]    = s.calib.emaAlpha;
            
            if (s.kind == SignalKind::ANALOG_IN || s.kind == SignalKind::ANALOG_OUT) {
                c["rawMin"]   = s.calib.rawMin;
                c["rawMax"]   = s.calib.rawMax;
                c["realMin"]  = s.calib.realMin;
                c["realMax"]  = s.calib.realMax;
            }
        }

        String out;
        serializeJson(doc, out);
        request->send(200, "application/json", out);
    });

    // =======================================================
    // POST/PUT /api/signals/update -> Actualizar calibración
    // Nota: AsyncWebServer maneja mejor los parámetros en rutas fijas
    // =======================================================
    server->on("/api/signals/update", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, data, len);

        if (error) {
            request->send(400, "application/json", "{\"error\":\"invalid_json\"}");
            return;
        }

        const char* id = doc["id"] | "";
        Signal* s = SignalMgr::getById(id);

        if (!s) {
            request->send(404, "application/json", "{\"error\":\"not_found\"}");
            return;
        }

        bool changed = false;

        if (doc.containsKey("offset")) {
            s->calib.offset = doc["offset"].as<float>();
            changed = true;
        }

        if (doc.containsKey("ema")) {
            s->calib.emaAlpha = doc["ema"].as<float>();
            changed = true;
        }

        if (changed) {
            // Reiniciar filtros para que el cambio sea inmediato
            s->calib.emaInit = false;

            // Persistencia en LittleFS/SD para que no se pierda al reiniciar
            saveSignalCalibration(*s);
            escribirLog("WEB: Calibracion actualizada para [%s]", s->id);
            
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
            request->send(304, "application/json", "{\"status\":\"no_change\"}");
        }
    });
}