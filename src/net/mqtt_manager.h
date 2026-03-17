#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <Client.h>

class MQTTManager {
public:
    static void init();
    static void loop();
    static bool publish(const char* topic, const char* payload, bool retain = false);
    static bool isConnected();
    static void onConnect();
    
    // Protocolo de intercambio con metadatos y latencia
    static void publishSignal(uint16_t id, float value, uint8_t status);
    
    // Gestión de interfaces (Failover)
    static void routeMQTT();
    static bool hasInternet();
    static void andleIncoming(char* topic, byte* payload, unsigned int length) 

private:
    static void onMqttMessage(char* topic, byte* payload, unsigned int length);
    static void reconnect();
    
    static Client* _activeNetClient;
    static PubSubClient _mqtt;
    static bool _wasConnected;
    static unsigned long _lastReconnectAttempt;
    static unsigned long _lastRouteCheck;
};

#endif