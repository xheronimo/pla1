#pragma once
#include <stdint.h>
#include "alarm/alarm_destination.h"

// ==================================================
// METADATA
// ==================================================
struct ConfigMeta {
    char id[32];
    char model[32];
    char fw[16];
    char build[32];
};

// ==================================================
// ESTADO GLOBAL (master switches)
// ==================================================
struct ConfigState {
    bool system = true;
    bool alarms = true;
    bool sms    = true;
};

// ==================================================
// NETWORK / MODEM
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
// MQTT
// ==================================================
struct ConfigMQTT {
    bool enabled;
    char host[64];
    uint16_t port;
    char user[32];
    char pass[32];
    char baseTopic[64];
};

// ==================================================
// TELEGRAM
// ==================================================
struct ConfigTelegram {
    bool enabled;
    char token[64];
};

// ==================================================
// RTC
// ==================================================
struct ConfigRTC {
    bool enabled;
    int timezone;
};

#define MAX_ALARM_DESTINATIONS 10



// ==================================================
// CONFIG GLOBAL
// ==================================================
struct Configuracion {

    ConfigMeta meta;
    ConfigState state;

    ConfigNetwork network;
    ConfigModem modem;

    ConfigMQTT mqtt;
    ConfigTelegram telegram;
    ConfigRTC rtc;

    AlarmDestination alarmDestinations[MAX_ALARM_DESTINATIONS];
};
