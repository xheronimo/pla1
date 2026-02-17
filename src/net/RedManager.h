#pragma once
#include <stdint.h>
#include <Arduino.h>

class RedManager
{
public:
    enum class NetworkState : uint8_t
    {
        NET_DOWN = 0,
        NET_CONNECTING,
        NET_READY,
        NET_ERROR
    };

    static void init();
    static void loop();

    static NetworkState getState();
    static bool isReady();

    static uint32_t getUptime();
    static uint32_t getErrorCount();
    static uint32_t getReconnectCount();

    // 👇 helpers legacy
    static const char* redActivaStr();
};
