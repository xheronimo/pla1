#pragma once
#include <stdint.h>

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

// ==================================================
// ALARM DESTINATIONS
// ==================================================
#define MAX_ALARM_DESTINATIONS 10

enum AlarmChannel : uint8_t {
    CH_NONE     = 0,
    CH_SMS      = 1 << 0,
    CH_TELEGRAM = 1 << 1,
    CH_MQTT     = 1 << 2
};

struct AlarmDestination {
    bool activo;
    uint8_t canales;      // AlarmChannel mask
    uint64_t grupos;      // AlarmGroup mask
    bool enviarRecuperacion;

    char nombre[24];

    // SMS
    char telefono[20];

    // Telegram
    int64_t telegramChatId;

    // MQTT
    char mqttTopic[64];
};

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
