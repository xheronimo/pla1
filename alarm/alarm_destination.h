#pragma once
#include <stdint.h>

#define MAX_ALARM_DESTINATIONS 10

enum AlarmChannel {
    CH_SMS      = 1 << 0,
    CH_TELEGRAM = 1 << 1,
    CH_MQTT     = 1 << 2
};

struct AlarmDestination {
    bool activo;
    uint64_t grupos;
    uint8_t canales;
    bool enviarRecuperacion;

    char nombre[24];
    char telefono[20];
    int64_t telegramChatId;
    char mqttTopic[64];
};

