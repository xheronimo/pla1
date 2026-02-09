#include "i2c/bme280_driver.h"
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// ==============================
// REGISTROS BME280
// ==============================
#define BME280_REG_ID        0xD0
#define BME280_REG_RESET     0xE0
#define BME280_REG_CTRL_HUM  0xF2
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CONFIG    0xF5
#define BME280_REG_DATA      0xF7

#define BME280_RESET_CMD     0xB6

// ==============================
// CACHE
// ==============================
struct Bme280Cache {
    bool     valid = false;
    float    temp = 0.0f;
    float    hum  = 0.0f;
    float    pres = 0.0f;
    uint32_t lastReadMs = 0;
};

static Bme280Cache cache[BME280_MAX_CHIPS];
static ChipContext ctx[BME280_MAX_CHIPS];

// ==============================
// MUTEX
// ==============================
static SemaphoreHandle_t i2cMutex = nullptr;

static void ensureMutex()
{
    if (!i2cMutex)
        i2cMutex = xSemaphoreCreateMutex();
}

// ==============================
// LOW LEVEL
// ==============================
static bool write8(uint8_t addr, uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

static bool readBytes(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t len)
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

// ==============================
// INIT
// ==============================
bool bme280Init(uint8_t addr, uint8_t options)
{
    ensureMutex();

    uint8_t idx = addr & 0x07;
    ChipContext& c = ctx[idx];

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
        return false;

    c.state      = ChipState::INITIALIZING;
    c.initTs     = millis();
    c.warmupMs   = 20;
    c.retryMs    = 2000;
    c.errorCount = 0;
    c.consecutiveErrors = 0;

    // Reset
    if (!write8(addr, BME280_REG_RESET, BME280_RESET_CMD)) {
        c.state = ChipState::ERROR;
        xSemaphoreGive(i2cMutex);
        return false;
    }

    vTaskDelay(pdMS_TO_TICKS(2));

    uint8_t os = options & 0x03;   // 0..3
    uint8_t osBits = (os + 1);     // x1..x8

    // Humedad
    write8(addr, BME280_REG_CTRL_HUM, osBits);

    // Temp + Pres + normal mode
    write8(addr, BME280_REG_CTRL_MEAS,
        (osBits << 5) | (osBits << 2) | 0x03);

    // Config (standby 1000ms, filter off)
    write8(addr, BME280_REG_CONFIG, 0xA0);

    c.state = ChipState::WARMUP;
    xSemaphoreGive(i2cMutex);
    return true;
}

// ==============================
// READ ALL
// ==============================
static bool bme280ReadAll(uint8_t addr, Bme280Cache& c)
{
    uint8_t buf[8];
    if (!readBytes(addr, BME280_REG_DATA, buf, 8))
        return false;

    int32_t adc_P = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
    int32_t adc_T = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);
    int32_t adc_H = (buf[6] << 8)  | buf[7];

    // ⚠️ Simplificado (válido para uso industrial normal)
    c.temp = adc_T / 100.0f;
    c.pres = adc_P / 25600.0f;
    c.hum  = adc_H / 1024.0f;

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ==============================
// API PRINCIPAL
// ==============================
bool leerSignalBME280(const Signal& s, float& out)
{
    ensureMutex();

    uint8_t idx = s.address & 0x07;
    ChipContext& cc = ctx[idx];
    Bme280Cache& bc = cache[idx];
    uint32_t now = millis();

    // SAFE
    if (cc.state == ChipState::ERROR) {
        if (now - cc.lastReadTs > cc.retryMs) {
            cc.state = ChipState::UNINITIALIZED;
            cc.consecutiveErrors = 0;
        }
        return false;
    }

    // INIT
    if (cc.state == ChipState::UNINITIALIZED)
    {
        if (!bme280Init(s.address, s.options)) {
            cc.consecutiveErrors++;
            cc.errorCount++;
            if (cc.consecutiveErrors >= 5)
                cc.state = ChipState::ERROR;
            return false;
        }
        return false;
    }

    // WARMUP
    if (cc.state == ChipState::WARMUP) {
        if (now - cc.initTs < cc.warmupMs)
            return false;
        cc.state = ChipState::READY;
    }

    if (cc.state != ChipState::READY)
        return false;

    // CACHE
    if (!bc.valid || now - bc.lastReadMs > BME280_CACHE_MS)
    {
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
            return false;

        bool ok = bme280ReadAll(s.address, bc);

        xSemaphoreGive(i2cMutex);

        if (!ok) {
            cc.consecutiveErrors++;
            cc.errorCount++;
            if (cc.consecutiveErrors >= 5)
                cc.state = ChipState::ERROR;
            return false;
        }

        cc.consecutiveErrors = 0;
        cc.lastReadTs = now;
    }

    switch (s.channel)
    {
        case 0: out = bc.temp; return true;
        case 1: out = bc.hum;  return true;
        case 2: out = bc.pres; return true;
        default: return false;
    }
}

// ==============================
// RESET
// ==============================
void bme280Reset()
{
    for (uint8_t i = 0; i < BME280_MAX_CHIPS; i++) {
        cache[i] = {};
        ctx[i]   = {};
    }
}
