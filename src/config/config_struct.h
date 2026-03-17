#pragma once
#include <stdint.h>
#include <Arduino.h>
#include "alarm/alarm_destinations.h" // Asegúrate de que este .h defina 'AlarmDestination'

// ==================================================
// METADATA (Información de versión y hardware)
// ==================================================
struct ConfigMeta {
    char id[32];
    char model[32];
    char fw[16];
    char build[32];
};

// ==================================================
// ESTADO GLOBAL (Master switches de bajo nivel)
// ==================================================
struct ConfigState {
    bool system = true;
    bool alarms = true;
    bool sms    = true;
};

// ==================================================
// NETWORK / MODEM (Conectividad física)
// ==================================================
struct ConfigNetwork {
    char apn[32];
    char user[32];
    char pass[32];
    char pin[8];
};

struct ConfigModem {
    bool enabled;
};

// ==================================================
// MQTT (Protocolo de telemetría)
// ==================================================
struct ConfigMQTT {
    bool enabled;
    char host[64];
    uint16_t port;
    char user[32];
    char pass[32];
    char clientId[32]; // Añadido para identificar el PLC en el broker
    char baseTopic[64];
};

// ==================================================
// TELEGRAM (Notificaciones push)
// ==================================================
struct ConfigTelegram {
    bool enabled;
    char token[64];
    char chatId[32]; // Añadido: ¿A quién enviamos el mensaje por defecto?
};

// ==================================================
// RTC & NTP (Gestión del tiempo)
// ==================================================
struct ConfigRTC {
    bool enabled;
    int timezone;
    bool ntpEnabled;       
    char ntpServer[64];    
    uint32_t syncInterval; 
};

// ==================================================
// MODO DE SISTEMA
// ==================================================
enum class SystemMode : uint8_t {
    NORMAL = 0,   // Operación total
    SAFE,         // Diagnóstico (Alarmas OFF)
    RECOVERY      // Mantenimiento (Web + Config mínima)
};

#define MAX_ALARM_DESTINATIONS 10

// ==================================================
// CONFIG GLOBAL (Estructura Maestra)
// ==================================================
struct Configuracion {

    ConfigMeta meta;
    ConfigState state;

    ConfigNetwork network;
    ConfigModem modem;

    ConfigMQTT mqtt;
    ConfigTelegram telegram;
    ConfigRTC rtc;

    // Array dinámico de destinos (SMS, MQTT, Email, etc.)
    AlarmDestination alarmDestinations[MAX_ALARM_DESTINATIONS];

    // --- Switches de Activación de Módulos ---
    bool enableAlarms;
    bool enablePersistence;
    bool enableMqtt;
    bool enableWeb;
    bool enableSms;
    bool enableTelegram;
    bool enableDisplay;
    
    SystemMode systemMode;
};