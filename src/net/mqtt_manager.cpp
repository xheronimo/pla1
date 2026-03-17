#include "net/mqtt_manager.h"
#include <WiFi.h>
#include <ETH.h>
#include <ArduinoJson.h>
#include <time.h>
#include <rom/rtc.h>

#include "lte/lte_manager.h" 
#include "signal/signal_manager.h"
#include "system/LogSystem.h"
#include "system/boot_reason.h"
#include "net/mqtt_topics.h"

// --- Definición de Tópicos Segmentados ---
// 1. Tópico Público/General: Para supervisión externa y estatus global
#define TOPIC_GLOBAL_HEARTBEAT "factory/status/heartbeat"
#define TOPIC_GLOBAL_BOOT      "factory/status/boot"

// 2. Tópicos Privados: Para intercambio de datos entre PLCs (P2P)
// Usamos una raíz privada para que el filtrado sea más rápido
#define TOPIC_PRIVATE_DATA     "p2p/node/%s/data"     // %s = ClientID
#define TOPIC_PRIVATE_SUB      "p2p/node/+/data"      // Suscripción comodín privada

// Variables estáticas
WiFiClient _wifiClientMqtt;
PubSubClient MQTTManager::_mqtt;
Client* MQTTManager::_activeNetClient = nullptr;
bool MQTTManager::_wasConnected = false;
unsigned long MQTTManager::_lastReconnectAttempt = 0;
unsigned long MQTTManager::_lastRouteCheck = 0;
unsigned long MQTTManager::_lastHeartbeat = 0;
String MQTTManager::_clientId = "";

void MQTTManager::init() {
    _clientId = "PLC_" + WiFi.macAddress();
    _clientId.replace(":", "");

    routeMQTT(); 
    _mqtt.setServer("192.168.1.10", 1883);
    _mqtt.setCallback(onMqttMessage);
    _mqtt.setBufferSize(2048);
    
    escribirLog("MQTT: Sistema iniciado. ID: %s", _clientId.c_str());
}

/**
 * @brief Publica el latido de vida en el Tópico General (Público)
 * Formato ligero para que el SCADA o Dashboard general lo procese rápido.
 */
void MQTTManager::publishHeartbeat() {
    if (!_mqtt.connected()) return;

    StaticJsonDocument<128> doc;
    doc["id"] = _clientId;
    doc["st"] = systemInSafeMode() ? 2 : 0;
    doc["rssi"] = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;

    String payload;
    serializeJson(doc, payload);
    
    // Enviamos a la red general de estatus
    _mqtt.publish(TOPIC_GLOBAL_HEARTBEAT, payload.c_str(), false);
    _lastHeartbeat = millis();
}

/**
 * @brief Publica señales en el Tópico Privado (Segmentado)
 * Solo lo procesan los PLCs interesados en los datos crudos.
 */
void MQTTManager::publishSignal(uint16_t id, float value, uint8_t status) {
    if (!_mqtt.connected()) return;

    StaticJsonDocument<256> doc;
    doc["t"] = time(nullptr);
    doc["st"] = status;
    
    JsonObject sigs = doc.createNestedObject("sig");
    if (id != 0) sigs[String(id)] = value;

    String payload;
    serializeJson(doc, payload);
    
    char privateTopic[64];
    snprintf(privateTopic, sizeof(privateTopic), TOPIC_PRIVATE_DATA, _clientId.c_str());
    
    _mqtt.publish(privateTopic, payload.c_str(), true); // Retain para datos P2P
}

void MQTTManager::onMqttMessage(char* topic, byte* payload, unsigned int length) {
    // FILTRADO RÁPIDO: Si el mensaje viene del tópico de Heartbeats, 
    // lo ignoramos aquí para ahorrar ciclos de procesamiento de señales.
    if (strcmp(topic, TOPIC_GLOBAL_HEARTBEAT) == 0) return;

    // Solo procesamos si el tópico pertenece a la red privada de datos
    if (strncmp(topic, "p2p/", 4) != 0) return;

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) return;

    if (doc.containsKey("t")) {
        long remoteTime = doc["t"];
        int remoteStatus = doc["st"] | 0;
        
        if (abs((long)time(nullptr) - remoteTime) > 10) return;

        JsonObject sigs = doc["sig"];
        for (JsonPair s : sigs) {
            uint16_t sigId = atoi(s.key().c_str());
            float val = s.value().as<float>();
            SignalQuality q = (remoteStatus == 0) ? SignalQuality::GOOD : SignalQuality::WARNING;
            SignalMgr::updateValue(sigId, val, q);
        }
    }
}

void MQTTManager::onConnect() {
    escribirLog("MQTT: Conectado. Suscribiendo a red privada.");
    
    // 1. Nos suscribimos SOLO a los datos de otros nodos (Red Privada)
    _mqtt.subscribe(TOPIC_PRIVATE_SUB);
    
    // 2. Suscripción a comandos de emergencia (Opcional)
    _mqtt.subscribe(MQTT_TOPIC_ALARM_ACK);
    
    publishBootEvent();
    publishHeartbeat();
}

void MQTTManager::publishBootEvent() {
    if (!_mqtt.connected()) return;

    StaticJsonDocument<256> doc;
    doc["id"] = _clientId;
    doc["event"] = "boot";
    doc["reason"] = rtc_get_reset_reason(0);

    String payload;
    serializeJson(doc, payload);
    _mqtt.publish(TOPIC_GLOBAL_BOOT, payload.c_str(), true);
}

void MQTTManager::routeMQTT() {
    Client* target = nullptr;
    if (ETH.linkUp()) target = &ETHClient;
    else if (WiFi.status() == WL_CONNECTED) target = &_wifiClientMqtt;
    else if (LTEMgr::isReady()) target = LTEMgr::getClient();

    if (target != _activeNetClient) {
        if (_mqtt.connected()) _mqtt.disconnect();
        _activeNetClient = target;
        if (_activeNetClient) _mqtt.setClient(*_activeNetClient);
    }
}

void MQTTManager::reconnect() {
    unsigned long now = millis();
    if (now - _lastReconnectAttempt > 5000) {
        _lastReconnectAttempt = now;
        if (_mqtt.connect(_clientId.c_str())) {
            _lastReconnectAttempt = 0;
            onConnect();
        }
    }
}

void MQTTManager::loop() {
    if (millis() - _lastRouteCheck > 30000) {
        _lastRouteCheck = millis();
        routeMQTT();
    }

    if (!_mqtt.connected()) {
        _wasConnected = false;
        reconnect();
    } else {
        if (!_wasConnected) _wasConnected = true;
        _mqtt.loop();

        if (millis() - _lastHeartbeat > 30000) {
            publishHeartbeat();
        }
    }
}

bool MQTTManager::isConnected() { return _mqtt.connected(); }

bool MQTTManager::publish(const char* topic, const char* payload, bool retain) {
    if (!_mqtt.connected()) return false;
    return _mqtt.publish(topic, payload, retain);
}

// Dentro de MqttManager.cpp -> handleIncoming
void MQTTManager::handleIncoming(char* topic, byte* payload, unsigned int length) {
    // ... (Logica de filtrado de tópicos y deserialización JSON) ...

    if (doc.containsKey("t")) {
        // ... (Validación de latencia de 10s que hicimos antes) ...

        JsonObject sigs = doc["sig"];
        for (JsonPair s : sigs) {
            uint16_t sigId = atoi(s.key().c_str());
            float val = s.value().as<float>();
            
            // Usamos remote = true para que el Watchdog la vigile
            SignalMgr::updateValue(sigId, val, SignalQuality::GOOD, true);
        }
    }
}