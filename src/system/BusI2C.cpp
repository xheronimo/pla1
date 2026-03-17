#include "BusI2C.h"
#include "hardware_defs.h"
#include "system/system_sync.h"

#include <Arduino.h>
#include <Wire.h>

// ================= CONFIG =================

#define I2C_TIMEOUT_MS     50
#define I2C_MAX_ERRORS     5

// =========================================

static uint32_t i2cErrors = 0;

// --------------------------------------------------

static void I2C_ResetBus()
{
    pinMode(I2C_SCL, OUTPUT);
    pinMode(I2C_SDA, OUTPUT);

    digitalWrite(I2C_SDA, HIGH);

    for (int i = 0; i < 9; i++) {
        digitalWrite(I2C_SCL, HIGH);
        delayMicroseconds(5);
        digitalWrite(I2C_SCL, LOW);
        delayMicroseconds(5);
    }

    Wire.begin(I2C_SDA, I2C_SCL);
}

// --------------------------------------------------

bool BusI2C_Init()
{
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setTimeOut(I2C_TIMEOUT_MS);

    // semI2C lo crea systemSyncInit()
    return semI2C != nullptr;
}

// --------------------------------------------------

static bool I2C_Lock(uint32_t timeout = I2C_TIMEOUT_MS)
{
    return xSemaphoreTake(semI2C, pdMS_TO_TICKS(timeout)) == pdTRUE;
}

static void I2C_Unlock()
{
    xSemaphoreGive(semI2C);
}

// --------------------------------------------------

bool BusI2C_Write(uint8_t addr, uint8_t data)
{
    if (!I2C_Lock()) return false;

    Wire.beginTransmission(addr);
    Wire.write(data);
    uint8_t err = Wire.endTransmission();

    I2C_Unlock();

    if (err != 0) {
        i2cErrors++;
        if (i2cErrors >= I2C_MAX_ERRORS) {
            I2C_ResetBus();
            i2cErrors = 0;
        }
        return false;
    }

    return true;
}

// --------------------------------------------------

bool BusI2C_WriteBytes(uint8_t addr, uint8_t* data, uint8_t len)
{
    if (!I2C_Lock()) return false;

    Wire.beginTransmission(addr);
    Wire.write(data, len);
    uint8_t err = Wire.endTransmission();

    I2C_Unlock();

    if (err != 0) {
        i2cErrors++;
        if (i2cErrors >= I2C_MAX_ERRORS) {
            I2C_ResetBus();
            i2cErrors = 0;
        }
        return false;
    }

    return true;
}

// --------------------------------------------------

bool BusI2C_Read(uint8_t addr, uint8_t* data, uint8_t len)
{
    if (!I2C_Lock()) return false;

    uint8_t readed = Wire.requestFrom(addr, len);

    if (readed != len) {
        I2C_Unlock();
        i2cErrors++;
        return false;
    }

    for (uint8_t i = 0; i < len; i++) {
        data[i] = Wire.read();
    }

    I2C_Unlock();
    return true;
}

// --------------------------------------------------

uint32_t BusI2C_GetErrors()
{
    return i2cErrors;
}
