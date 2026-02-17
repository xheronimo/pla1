#include "modbus_traffic.h"
#include <Arduino.h>

static uint32_t g_lastFrameMs = 0;
static const uint16_t MODBUS_MIN_GAP_MS = 5;

bool modbusTrafficAllow()
{
    uint32_t now = millis();

    if (now - g_lastFrameMs < MODBUS_MIN_GAP_MS)
        return false;

    g_lastFrameMs = now;
    return true;
}
