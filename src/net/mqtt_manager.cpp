#include "mqtt_manager.h"

#include <WiFi.h>
#include <PubSubClient.h>

#include "mqtt_commands.h"
#include "mqtt_snapshot.h"
#include "net/mqtt_topics.h"
#include "ArduinoJson.h"
#include "system/boot_reason.h"

// --------------------------------------------------
// Cliente MQTT
// --------------------------------------------------
static WiFiClient wifiClient;
static PubSubClient mqtt(wifiClient);

// --------------------------------------------------
// Configuración
// --------------------------------------------------
static const char* MQTT_HOST      = "192.168.1.10";
static const uint16_t MQTT_PORT   = 1883;
static const char* MQTT_CLIENT_ID = "esp32_alarm";

// --------------------------------------------------
// Estado interno
// --------------------------------------------------
static bool wasConnected = false;

// --------------------------------------------------
// Callback de mensajes entrantes
// --------------------------------------------------
static void onMqttMessage(char* topic, byte* payload, unsigned int length)
{
    static char msg[256];
    if (length >= sizeof(msg))
        return;

    memcpy(msg, payload, length);
    msg[length] = '\0';

    // 👉 Despacho a comandos MQTT (ACK, etc.)
    mqttOnMessage(topic, msg);
}

// --------------------------------------------------
// Inicialización
// --------------------------------------------------
void MQTTManager::init()
{
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setCallback(onMqttMessage);
}

// --------------------------------------------------
// Reconexión
// --------------------------------------------------
static void reconnect()
{
    if (mqtt.connected())
        return;

    mqtt.connect(MQTT_CLIENT_ID);
}

// --------------------------------------------------
// Loop principal
// --------------------------------------------------
void MQTTManager::loop()
{
    if (!mqtt.connected())
    {
        wasConnected = false;
        reconnect();
        return;
    }

    // 🔥 Conexión nueva detectada
    if (!wasConnected)
    {
        wasConnected = true;
        MQTTManager::onConnect();
    }

    mqtt.loop();
}

// --------------------------------------------------
// Publicar mensaje
// --------------------------------------------------
bool MQTTManager::publish(
    const char* topic,
    const char* payload,
    bool retain
)
{
    if (!mqtt.connected())
        return false;

    return mqtt.publish(topic, payload, retain);
}

// --------------------------------------------------
// Estado de conexión
// --------------------------------------------------
bool MQTTManager::isConnected()
{
    
    return mqtt.connected();

}

// --------------------------------------------------
// Acciones al conectar
// --------------------------------------------------
void MQTTManager::onConnect()
{
    // Suscribirse a comandos
    mqtt.subscribe(MQTT_TOPIC_ALARM_ACK);

    // Enviar snapshot completo de alarmas
    mqttPublishAlarmSnapshot();
    publishBootEvent();
}


void publishBootEvent()
{
    if (!MQTTManager::isConnected())
        return;

    JsonDocument doc;
    doc["event"] = "boot";
    doc["uptime"] = millis();
    doc["wdtCount"] = g_wdtResetCount;

    if (g_lastResetWdt)
        doc["reason"] = "watchdog";
    else if (g_lastResetPower)
        doc["reason"] = "poweron";
    else if (g_lastResetSoftware)
        doc["reason"] = "software";
    else
        doc["reason"] = "other";

    String payload;
    serializeJson(doc, payload);

    MQTTManager::publish("system/boot", payload.c_str(), true);
}
