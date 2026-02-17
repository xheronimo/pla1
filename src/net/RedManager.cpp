#include "RedManager.h"
#include "system/WatchdogManager.h"
#include <Arduino.h>

#define NETWORK_RETRY_MS 5000

static RedManager::NetworkState g_state = RedManager::NetworkState::NET_DOWN;
static uint32_t g_startMs = 0;
static uint32_t g_lastAttemptMs = 0;
static uint32_t g_errorCount = 0;
static uint32_t g_reconnectCount = 0;

void RedManager::init()
{
    g_startMs = millis();
    g_lastAttemptMs = 0;
    g_errorCount = 0;
    g_reconnectCount = 0;

    g_state = NetworkState::NET_CONNECTING;
    watchdogRegister(WDT_NET);
}

void RedManager::loop()
{
    uint32_t now = millis();

    switch (g_state)
    {
        case NetworkState::NET_CONNECTING:
            if (now - g_lastAttemptMs >= NETWORK_RETRY_MS)
            {
                g_lastAttemptMs = now;

                bool ok = true; // aquí Ethernet/WiFi real

                if (ok)
                    g_state = NetworkState::NET_READY;
                else
                {
                    g_errorCount++;
                    g_reconnectCount++;
                    g_state = NetworkState::NET_ERROR;
                }
            }
            break;

        case NetworkState::NET_READY:
            break;

        case NetworkState::NET_ERROR:
            if (now - g_lastAttemptMs >= NETWORK_RETRY_MS)
                g_state = NetworkState::NET_CONNECTING;
            break;

        default:
            break;
    }

    watchdogKick(WDT_NET);
}

RedManager::NetworkState RedManager::getState()
{
    return g_state;
}

bool RedManager::isReady()
{
    return g_state == NetworkState::NET_READY;
}

uint32_t RedManager::getUptime()
{
    return millis() - g_startMs;
}

uint32_t RedManager::getErrorCount()
{
    return g_errorCount;
}

uint32_t RedManager::getReconnectCount()
{
    return g_reconnectCount;
}

const char* RedManager::redActivaStr()
{
    return (g_state == NetworkState::NET_READY) ? "OK" : "OFF";
}
