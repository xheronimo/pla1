// alarm_destination.h
#pragma once
#include <stdint.h>
#include "alarm_defs.h"

enum AlarmChannel : uint8_t {
    CH_NONE     = 0,
    CH_MQTT     = 1 << 0,
    CH_SMS      = 1 << 1,
    CH_TELEGRAM = 1 << 2
};

struct AlarmDestination {
    bool activo;
    uint8_t canales;
    uint64_t grupos;
    bool enviarRecuperacion;

    char nombre[24];

    // SMS
    char telefono[20];

    // Telegram
    int64_t telegramChatId;

    // MQTT
    char mqttTopic[64];
};


extern AlarmDestination alarmDestinations[MAX_ALARM_DESTINATIONS];
extern uint8_t alarmDestinationCount;

