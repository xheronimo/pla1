#include "i2c/pcf_driver.h"
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// =================================================
// CACHES SEPARADAS
// =================================================
static PcfCache cacheUser[PCF_MAX_CHIPS];
static PcfCache cacheSystem[PCF_MAX_CHIPS];

// =================================================
// MUTEX I2C
// =================================================
static SemaphoreHandle_t i2cMutex = nullptr;

static void ensureMutex()
{
    if (!i2cMutex)
        i2cMutex = xSemaphoreCreateMutex();
}

// =================================================
// LOW LEVEL READ
// =================================================
static bool readPCF(uint8_t addr, I2CDevice chip, uint16_t& out)
{
    if (chip == I2CDevice::PCF8574)
    {
        Wire.requestFrom(addr, (uint8_t)1);
        if (Wire.available() < 1) return false;
        out = Wire.read();
        return true;
    }

    if (chip == I2CDevice::PCF8575)
    {
        Wire.requestFrom(addr, (uint8_t)2);
        if (Wire.available() < 2) return false;
        uint8_t lo = Wire.read();
        uint8_t hi = Wire.read();
        out = (hi << 8) | lo;
        return true;
    }

    return false;
}

// =================================================
// LOW LEVEL WRITE
// =================================================
static bool writePCF(uint8_t addr, I2CDevice chip, uint16_t value)
{
    Wire.beginTransmission(addr);

    if (chip == I2CDevice::PCF8574)
    {
        Wire.write((uint8_t)value);
    }
    else if (chip == I2CDevice::PCF8575)
    {
        Wire.write(value & 0xFF);
        Wire.write(value >> 8);
    }
    else
        return false;

    return Wire.endTransmission() == 0;
}

// =================================================
// COMMON READ
// =================================================
static bool leerPCF(const Signal& s, PcfCache* cacheArray, float& out)
{
    ensureMutex();

    uint8_t idx = s.address & 0x07;
    if (idx >= PCF_MAX_CHIPS) return false;

    PcfCache& c = cacheArray[idx];
    uint32_t now = millis();

    if (c.errorCount >= PCF_MAX_ERRORS)
    {
        if (now - c.lastReadMs < 2000)
            return false;
        c.errorCount = 0;
    }

    if (!c.valid || now - c.lastReadMs > PCF_CACHE_MS)
    {
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
            return false;

        bool ok = readPCF(s.address, s.chip, c.value);

        xSemaphoreGive(i2cMutex);

        if (!ok)
        {
            c.errorCount++;
            c.valid = false;
            c.lastReadMs = now;
            return false;
        }

        c.valid = true;
        c.lastReadMs = now;
        c.errorCount = 0;
    }

    uint8_t bit = s.channel;

    if ((s.chip == I2CDevice::PCF8574 && bit > 7) ||
        (s.chip == I2CDevice::PCF8575 && bit > 15))
        return false;

    bool level = (c.value >> bit) & 0x01;
    out = level ? 1.0f : 0.0f;
    return true;
}

// =================================================
// COMMON WRITE
// =================================================
static bool escribirPCF(const Signal& s, PcfCache* cacheArray, float value)
{
    ensureMutex();

    uint8_t idx = s.address & 0x07;
    if (idx >= PCF_MAX_CHIPS) return false;

    PcfCache& c = cacheArray[idx];

    uint8_t bit = s.channel;
    bool level = (value != 0.0f);

    if ((s.chip == I2CDevice::PCF8574 && bit > 7) ||
        (s.chip == I2CDevice::PCF8575 && bit > 15))
        return false;

    // Inicializar cache si no existe
    if (!c.valid)
    {
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
            return false;

        bool ok = readPCF(s.address, s.chip, c.value);
        xSemaphoreGive(i2cMutex);

        if (!ok)
        {
            c.errorCount++;
            return false;
        }
        c.valid = true;
    }

    // Modificar bit
    if (level)
        c.value |= (1 << bit);
    else
        c.value &= ~(1 << bit);

    // Escribir todo el puerto
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
        return false;

    bool ok = writePCF(s.address, s.chip, c.value);
    xSemaphoreGive(i2cMutex);

    if (!ok)
    {
        c.errorCount++;
        c.valid = false;
        return false;
    }

    c.lastReadMs = millis();
    c.errorCount = 0;
    return true;
}

// =================================================
// API USER
// =================================================
bool leerSignalPCF_User(const Signal& s, float& out)
{
    return leerPCF(s, cacheUser, out);
}

bool escribirSignalPCF_User(const Signal& s, float value)
{
    return escribirPCF(s, cacheUser, value);
}

// =================================================
// API SYSTEM
// =================================================
bool leerSignalPCF_System(const Signal& s, float& out)
{
    return leerPCF(s, cacheSystem, out);
}

bool escribirSignalPCF_System(const Signal& s, float value)
{
    return escribirPCF(s, cacheSystem, value);
}

// =================================================
void pcfResetCaches()
{
    for (uint8_t i = 0; i < PCF_MAX_CHIPS; i++)
    {
        cacheUser[i]   = {};
        cacheSystem[i] = {};
    }
}
