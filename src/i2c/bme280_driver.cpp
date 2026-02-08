#include "i2c/bme280_driver.h"
#include <Wire.h>

// ==============================
// REGISTROS
// ==============================
#define BME280_REG_CTRL_HUM   0xF2
#define BME280_REG_CTRL_MEAS  0xF4
#define BME280_REG_CONFIG     0xF5
#define BME280_REG_DATA       0xF7

// ==============================
// CACHE
// ==============================
static Bme280Cache bmeCache[BME280_MAX_CHIPS];

// ==============================
// LOW LEVEL
// ==============================
static bool i2cRead(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t len)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(addr, len) != len)
        return false;

    for (uint8_t i = 0; i < len; i++)
        buf[i] = Wire.read();

    return true;
}

static bool i2cWrite(uint8_t addr, uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

// ==============================
// INIT + READ ALL
// ==============================
static bool bme280ReadAll(uint8_t addr, Bme280Cache& c)
{
    // Oversampling x1, modo normal
    i2cWrite(addr, BME280_REG_CTRL_HUM, 0x01);
    i2cWrite(addr, BME280_REG_CTRL_MEAS, 0x27);
    i2cWrite(addr, BME280_REG_CONFIG, 0xA0);

    delay(10); // BME necesita pequeño settling

    uint8_t buf[8];
    if (!i2cRead(addr, BME280_REG_DATA, buf, 8))
        return false;

    // Raw
    int32_t rawPress = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
    int32_t rawTemp  = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);
    int32_t rawHum   = (buf[6] << 8)  | buf[7];

    // ⚠️ Simplificado (asume calibración estándar)
    c.temperature = rawTemp / 100.0f;
    c.humidity    = rawHum  / 1024.0f;
    c.pressure    = rawPress / 25600.0f;

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ==============================
// API SIGNAL
// ==============================
bool leerSignalBME280(const Signal& s, float& out)
{
    uint8_t idx = s.address & 0x07;
    Bme280Cache& c = bmeCache[idx];

    if (!c.valid || millis() - c.lastReadMs > BME280_CACHE_MS)
    {
        if (!bme280ReadAll(s.address, c))
            return false;
    }

    switch (s.channel)
    {
        case 0: out = c.temperature; return true;
        case 1: out = c.humidity;    return true;
        case 2: out = c.pressure;    return true;
        default: return false;
    }
}

// ==============================
// RESET
// ==============================
void bme280ResetCaches()
{
    for (uint8_t i = 0; i < BME280_MAX_CHIPS; i++)
        bmeCache[i] = {};
}
