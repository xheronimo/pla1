#include "i2c_watchdog.h"
#include "board/board_config.h"
#include "i2c_metrics.h"
#include <Arduino.h>
#include <Wire.h>

#define I2C_BUS_RESET_COOLDOWN_MS   10000
#define I2C_WATCHDOG_TIMEOUT_MS     30000

static uint32_t g_lastResetMs = 0;
static uint32_t g_lastActivityMs = 0;

void i2cWatchdogInit()
{
    g_lastActivityMs = millis();
}

void i2cWatchdogActivity()
{
    g_lastActivityMs = millis();
}

bool i2cBusIsStuck()
{
    pinMode(I2C_SDA_PIN, INPUT_PULLUP);
    pinMode(I2C_SCL_PIN, INPUT_PULLUP);
    delayMicroseconds(5);
    return digitalRead(I2C_SDA_PIN) == LOW;
}

void i2cBusReset()
{
    uint32_t now = millis();

    if (now - g_lastResetMs < I2C_BUS_RESET_COOLDOWN_MS)
        return;

    if (!i2cBusIsStuck())
        return;

    pinMode(I2C_SCL_PIN, OUTPUT_OPEN_DRAIN);
    pinMode(I2C_SDA_PIN, OUTPUT_OPEN_DRAIN);

    digitalWrite(I2C_SDA_PIN, HIGH);

    for (int i = 0; i < 9; i++)
    {
        digitalWrite(I2C_SCL_PIN, LOW);
        delayMicroseconds(5);
        digitalWrite(I2C_SCL_PIN, HIGH);
        delayMicroseconds(5);
    }

    digitalWrite(I2C_SDA_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(I2C_SCL_PIN, HIGH);
    delayMicroseconds(5);
    digitalWrite(I2C_SDA_PIN, HIGH);

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    g_lastResetMs = now;
}

void i2cWatchdogCheck()
{
    if (millis() - g_lastActivityMs > I2C_WATCHDOG_TIMEOUT_MS)
        i2cBusReset();
}
