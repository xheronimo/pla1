#include "i2c_metrics.h"
#include <Wire.h>
#include <Arduino.h>

#define I2C_GLOBAL_ERROR_THRESHOLD  20
#define I2C_SPEED_HIGH  400000
#define I2C_SPEED_LOW   100000

static uint32_t g_totalOps = 0;
static uint32_t g_totalErrors = 0;
static uint32_t g_totalResets = 0;
static uint8_t  g_errorStreak = 0;
static uint32_t g_currentSpeed = I2C_SPEED_HIGH;

void i2cMetricsInit()
{
    i2cSetSpeed(I2C_SPEED_HIGH);
}

void i2cSetSpeed(uint32_t hz)
{
    g_currentSpeed = hz;
    Wire.setClock(hz);
}

void i2cNotifySuccess()
{
    g_errorStreak = 0;
    g_totalOps++;
}

void i2cNotifyError()
{
    g_errorStreak++;
    g_totalErrors++;
    g_totalOps++;

    if (g_errorStreak >= I2C_GLOBAL_ERROR_THRESHOLD)
    {
        if (g_currentSpeed == I2C_SPEED_HIGH)
            i2cSetSpeed(I2C_SPEED_LOW);
    }
}

uint32_t i2cGetTotalOps()    { return g_totalOps; }
uint32_t i2cGetTotalErrors() { return g_totalErrors; }
uint32_t i2cGetTotalResets() { return g_totalResets; }
uint32_t i2cGetCurrentSpeed(){ return g_currentSpeed; }

float i2cGetBusHealth()
{
    if (g_totalOps == 0) return 100.0f;
    float errRate = (float)g_totalErrors / (float)g_totalOps;
    float health = 100.0f - errRate * 100.0f;
    return health < 0 ? 0 : health;
}
