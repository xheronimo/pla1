#include "modbus/modbus_rollback.h"
#include "modbus/modbus_scheduler.h"
#include "system/LogSystem.h"
#include <Preferences.h>
#include <ArduinoJson.h>

namespace ModbusRollback {

    static uint32_t lastStableUpdate = 0; 
    static uint32_t failStart = 0;        
    static uint32_t lastProblemTs = 0;    

    void resetStabilityTimer() {
        failStart = 0;
        lastProblemTs = millis();
        lastStableUpdate = millis();
        LOG_INF("[Rollback] Temporizadores de estabilidad reiniciados.");
    }

    bool isBusStable(uint32_t thresholdMs) {
        // Consultamos el estado al Scheduler
        if (strcmp(ModbusScheduler::getBusStatus(), "OK") != 0) return false;
        return (millis() - lastProblemTs >= thresholdMs);
    }

    void saveStableSnapshot() {
        Preferences prefs;
        if (!prefs.begin("mb_stable", false)) return;

        JsonDocument doc;
        JsonArray array = doc.to<JsonArray>();

        const auto& devices = ModbusScheduler::getDevices();
        if (devices.empty()) {
            prefs.end();
            return;
        }

        for (const auto& dev : devices) {
            JsonObject obj = array.add<JsonObject>();
            obj["id"] = dev.id;
            obj["name"] = dev.name;
            obj["fp"] = dev.fingerprint;
            obj["poll"] = dev.pollMs;
        }

        String output;
        serializeJson(doc, output);
        prefs.putString("config_json", output);
        prefs.end();
        
        lastStableUpdate = millis();
        LOG_INF("[Rollback] Snapshot estable guardado (%d dispositivos)", devices.size());
    }

    bool performRollback() {
        Preferences prefs;
        if (!prefs.begin("mb_stable", true)) return false;

        String input = prefs.getString("config_json", "");
        prefs.end();

        if (input.length() == 0) return false;

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, input);
        if (error) return false;

        LOG_ERR("[Rollback] ¡Bus caído! Restaurando última configuración estable...");
        
        ModbusScheduler::clearDevices();
        JsonArray array = doc.as<JsonArray>();
        for (JsonObject obj : array) {
            ModbusDevice dev(obj["id"], obj["name"], obj["fp"]);
            dev.pollMs = obj["poll"];
            ModbusScheduler::addDevice(dev);
        }

        resetStabilityTimer();
        return true;
    }

    void monitorStability() {
        const char* status = ModbusScheduler::getBusStatus();
        uint32_t now = millis();

        if (strcmp(status, "OK") != 0) {
            lastProblemTs = now; // Registramos que el bus no está perfecto

            if (strcmp(status, "DOWN") == 0) {
                if (failStart == 0) failStart = now;
                if (now - failStart > 15000) { // 15s sin ninguna respuesta
                    performRollback();
                    failStart = 0; 
                }
            }
        } else {
            failStart = 0;
            // Si todo va bien durante 1 minuto, guardamos el éxito
            if (now - lastStableUpdate > 60000) {
                saveStableSnapshot();
            }
        }
    }

} // namespace ModbusRollback