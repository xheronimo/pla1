#include "web/web_manager.h"
#include <ESPAsyncWebServer.h>
#include <SD.h>

// --- APIs de Datos y Alarmas ---
#include "web_api_alarms.h"
#include "web_api_alarm_ack.h"
#include "web_api_signals.h"

// --- APIs de Diagnóstico de Hardware ---
#include "web_api_i2c_health.h"
#include "web_api_i2c_status.h"
#include "web_api_modbus_health.h"
#include "web_api_watchdog_health.h"

// --- APIs de Estado de Sistema ---
#include "web_api_health.h"
#include "web_api_system_health.h"
#include "web_api_system_boot.h"
#include "web_api_system_fault.h"
#include "web_api_system_reset.h"

// --- APIs de Configuración Dinámica I2C ---
#include "api_i2c_scan.h"
#include "api_i2c_drivers.h"
#include "api_i2c_create.h"

namespace WebMgr {

    void init(AsyncWebServer* srv) {
        
        // 1. REGISTRO DE MÓDULOS API (Ordenado por categorías)
        
        // --- Datos de proceso ---
        registerSignalApi(srv);         // /api/signals
        registerAlarmsApi(srv);         // /api/alarms
        registerAlarmAckApi(srv);       // /api/alarm/ack

        // --- Diagnóstico de Hardware (Buses) ---
        registerI2CHealthAPI(srv);      // /api/i2c/health (Chip a chip)
        registerI2CStatusAPI(srv);      // /api/i2c/status (Global del bus)
        registerModbusHealthApi(srv);   // /api/modbus/health
        registerWatchdogHealthAPI(srv); // /api/watchdog/health (13 canales)

        // --- Configuración y Detección ---
        registerI2CScanAPI(srv);        // /api/i2c/scan
        registerI2CDriversAPI(srv);     // /api/i2c/drivers
        registerI2CCreateAPI(srv);      // /api/i2c/create

        // --- Estado Vital del Sistema ---
        registerHealthApi(srv);         // /api/health (Resumen)
        registerSystemHealthAPI(srv);   // /api/system/health (Profundo)
        registerSystemBootApi(srv);     // /api/system/boot
        registerSystemFaultApi(srv);    // /api/system/fault
        registerSystemResetAPI(srv);    // /api/system/reset y /api/system/reset_wdt

        // 2. SERVIR INTERFAZ WEB DESDE SD
        // Se encarga de servir index.html, CSS y JS
        srv->serveStatic("/", SD, "/www/").setDefaultFile("index.html");

        // 3. MANEJO DE ERROR 404 (Recurso no encontrado)
        srv->onNotFound([](AsyncWebServerRequest *request){
            request->send(404, "application/json", "{\"error\":\"Not Found\"}");
        });

        // 4. INICIO DEL SERVIDOR
        srv->begin();
    }
}