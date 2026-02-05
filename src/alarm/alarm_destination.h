#pragma once
#include <stdint.h>

#define MAX_ALARM_DESTINATIONS 10

enum AlarmChannel : uint8_t {
    CH_NONE     = 0,
    CH_SMS      = 1 << 0,
    CH_TELEGRAM = 1 << 1,
    CH_MQTT     = 1 << 2
};

struct AlarmDestination {
    bool activo = false;
    uint8_t canales = CH_NONE;
    uint64_t grupos = 0;
    bool enviarRecuperacion = false;

    char nombre[24] = {0};

    // SMS
    char telefono[20] = {0};

    // Telegram
    int64_t telegramChatId = 0;

    // MQTT
    char mqttTopic[64] = {0};
};
