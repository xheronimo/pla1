#include "net/NetworkManager.h"
#include <WiFi.h>
#include <ETH.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ESP32Ping.h>

#include "lte/lte_manager.h"
#include "signal/signal_manager.h"
#include "system/LogSystem.h"
#include "net/mqtt_manager.h"

// --- Miembros Estáticos: Servicios Web ---
AsyncWebSocket NetworkMgr::_ws("/ws");
unsigned long NetworkMgr::_apStartTime = 0;
unsigned long NetworkMgr::_apDuration = 0;
bool NetworkMgr::_apTimerEnabled = false;

// --- Miembros Estáticos: Failover y Ruta ---
NetInterface NetworkMgr::_currentActiveInterface = NetInterface::NONE;
bool NetworkMgr::_isTransitioning = false;
unsigned long NetworkMgr::_lastRouteCheck = 0;

/**
 * @brief Inicialización global de red y servicios
 */
void NetworkMgr::init(AsyncWebServer* server) {
    escribirLog("NET: Iniciando NetworkManager consolidado...");

    // 1. Configurar WebSockets
    _ws.onEvent(onWsEvent);
    server->addHandler(&_ws);
    
    // 2. Ruta del Dashboard en SD
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        if (SD.exists("/www/index.html")) {
            request->send(SD, "/www/index.html", "text/html");
        } else {
            request->send(404, "text/plain", "Dashboard no encontrado en SD");
        }
    });

// 1. Iniciar Escaneo de dispositivos Modbus
    server->on("/api/modbus/scan", HTTP_POST, [](AsyncWebServerRequest *request){
        if (ModbusProvisioning::isScanning()) {
            request->send(400, "application/json", "{\"error\":\"Escaneo en curso\"}");
        } else {
            // Asumimos 9600 bps por defecto para el escaneo inicial
            ModbusProvisioning::startScan(9600); 
            request->send(200, "application/json", "{\"status\":\"Iniciado\"}");
        }
    });

    // 2. Consultar progreso y métricas de carga
    server->on("/api/modbus/status", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<256> doc;
        doc["scanning"] = ModbusProvisioning::isScanning();
        doc["progress"] = ModbusProvisioning::getProgress();
        // Nota: getCurrentScanId() debe ser público en tu .h
        // doc["current_id"] = ModbusProvisioning::getCurrentScanId(); 
        
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    // 3. Asignar nuevo ID (Provisioning)
    server->on("/api/modbus/assign", HTTP_POST, [](AsyncWebServerRequest *request){
        if (!request->hasParam("old") || !request->hasParam("new")) {
            request->send(400, "application/json", "{\"error\":\"Faltan parametros\"}");
            return;
        }
        
        uint8_t oldId = request->getParam("old")->value().toInt();
        uint8_t newId = request->getParam("new")->value().toInt();
        
        // regIdConfig suele ser el registro 0x0101 en muchos sensores industriales
        if (ModbusProvisioning::changeIdSafe(oldId, newId, 0x0101)) {
            request->send(200, "application/json", "{\"status\":\"ID actualizado\"}");
        } else {
            request->send(500, "application/json", "{\"error\":\"Fallo de comunicacion o ID ocupado\"}");
        }
    });


    // 3. Lanzar Tareas de RTOS en Core 0
    xTaskCreatePinnedToCore(monitorTask, "NetMon", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(recoveryTask, "NetRecov", 4096, NULL, 1, NULL, 0);
}

// ================================================================
// SECCIÓN 1: SERVICIOS WEB Y BROADCAST
// ================================================================

void NetworkMgr::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                          AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        client->text("{\"msg\":\"PLC Online\",\"type\":\"auth\"}");
    }
}

void NetworkMgr::broadcastSignals() {
    if (_ws.count() == 0) return; 

    StaticJsonDocument<2048> doc;
    doc["type"] = "update";
    JsonArray sigs = doc.createNestedArray("sigs");

    // Obtenemos todas las señales procesadas
    auto allSignals = SignalMgr::getAll(); 
    for (auto &sig : allSignals) {
        JsonObject s = sigs.createNestedObject();
        s["id"] = sig.id;
        s["v"] = sig.value;
        s["q"] = (int)sig.quality;
    }

    String output;
    serializeJson(doc, output);
    _ws.textAll(output);
}

void NetworkMgr::activateAPTimer(int minutes) {
    _apDuration = (unsigned long)minutes * 60 * 1000;
    _apStartTime = millis();
    _apTimerEnabled = true;
    escribirLog("NET: AP temporal activo (%d min)", minutes);
}

void NetworkMgr::manageAPSecurity() {
    if (!_apTimerEnabled) return;

    // Resetear timer si hay actividad de estaciones
    if (WiFi.softAPgetStationNum() > 0) {
        _apStartTime = millis(); 
        return;
    }

    if (millis() - _apStartTime > _apDuration) {
        escribirLog("NET: Cerrando AP por seguridad/inactividad.");
        WiFi.softAPdisconnect(true);
        _apTimerEnabled = false;
    }
}

// ================================================================
// SECCIÓN 2: FAILOVER, MONITORIZACIÓN Y RUTAS
// ================================================================

void NetworkMgr::monitorTask(void* pv) {
    for(;;) {
        // Ejecutar seguridad de AP
        manageAPSecurity();

        // Enviar datos a la web cada 500ms (si hay clientes)
        broadcastSignals();

        // Verificar salud de la conexión a internet cada 5 segundos
        if (!_isTransitioning && _currentActiveInterface != NetInterface::NONE) {
            if (!Ping.ping("8.8.8.8", 1)) {
                escribirLog("NET: Gateway inaccesible. Iniciando Failover...");
                handleFailover();
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void NetworkMgr::recoveryTask(void* pv) {
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(30000)); // Intentar recuperar interfaces mejores cada 30s

        if (_isTransitioning) continue;

        // Prioridad 1: Intentar volver a Ethernet
        if (_currentActiveInterface != NetInterface::ETHERNET) {
            if (ETH.linkUp()) {
                if (Ping.ping("8.8.8.8", 2)) {
                    escribirLog("NET: Ethernet recuperado. Abandonando rutas inferiores.");
                    switchInterface(NetInterface::ETHERNET);
                    continue;
                }
            }
        }

        // Prioridad 2: Si estamos en 4G, intentar volver a WiFi
        if (_currentActiveInterface == NetInterface::CELLULAR_4G) {
            if (WiFi.status() == WL_CONNECTED && Ping.ping("8.8.8.8", 1)) {
                escribirLog("NET: WiFi estable detectado. Cerrando enlace 4G.");
                switchInterface(NetInterface::WIFI);
            }
        }
    }
}

void NetworkMgr::handleFailover() {
    _isTransitioning = true;
    NetInterface best = NetInterface::NONE;

    if (ETH.linkUp() && Ping.ping("8.8.8.8", 1)) {
        best = NetInterface::ETHERNET;
    } else if (WiFi.status() == WL_CONNECTED && Ping.ping("8.8.8.8", 1)) {
        best = NetInterface::WIFI;
    } else if (LTEMgr::isReady()) {
        best = NetInterface::CELLULAR_4G;
    }

    if (best != _currentActiveInterface) {
        switchInterface(best);
    }
    _isTransitioning = false;
}

void NetworkMgr::switchInterface(NetInterface newIface) {
    _currentActiveInterface = newIface;
    
    // Actualizar el cliente MQTT para que use la nueva interfaz física
    MQTTManager::routeMQTT();
    
    escribirLog("NET: Ruta de datos conmutada a interfaz [%d]", (int)newIface);
}

void NetworkMgr::checkInterfaceHealth(NetInterface iface) {
    // Verificación P2P: ¿Vemos a la otra placa pero no a internet?
    bool canSeeLocal = Ping.ping("192.168.1.50"); // IP de la placa compañera
    bool canSeeInternet = Ping.ping("8.8.8.8");

    if (canSeeLocal && !canSeeInternet) {
        escribirLog("NET: Escenario LOCAL_ONLY detectado en interfaz %d", (int)iface);
    }
}