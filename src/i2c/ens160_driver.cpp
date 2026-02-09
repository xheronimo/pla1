#include "i2c/ens160_driver.h"
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// ==============================
// REGISTROS ENS160
// ==============================
#define ENS160_REG_PART_ID     0x00
#define ENS160_REG_OPMODE     0x10
#define ENS160_REG_TEMP_IN    0x13
#define ENS160_REG_RH_IN      0x15

#define ENS160_REG_DATA_AQI   0x21
#define ENS160_REG_DATA_TVOC  0x22
#define ENS160_REG_DATA_ECO2  0x24

// ==============================
// MODOS
// ==============================
#define ENS160_OPMODE_RESET   0xF0
#define ENS160_OPMODE_IDLE    0x01
#define ENS160_OPMODE_STD     0x02

// ==============================
// CACHE
// ==============================
struct Ens160Cache {
    bool     valid = false;
    uint16_t aqi   = 0;
    uint16_t tvoc  = 0;
    uint16_t eco2  = 0;
    uint32_t lastReadMs = 0;
};

static Ens160Cache cache[ENS160_MAX_CHIPS];
static ChipContext ctx[ENS160_MAX_CHIPS];

// ==============================
// MUTEX I2C
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
static bool i2cWrite8(uint8_t addr, uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

static bool i2cRead16(uint8_t addr, uint8_t reg, uint16_t& out)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(addr, (uint8_t)2) != 2)
        return false;

    out = Wire.read() | (Wire.read() << 8);
    return true;
}

// ==============================
// INIT
// ==============================
bool ens160Init(uint8_t addr, uint8_t options)
{
    ensureMutex();

    uint8_t idx = addr & 0x07;
    ChipContext& c = ctx[idx];

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
        return false;

    c.state      = ChipState::INITIALIZING;
    c.initTs     = millis();
    c.warmupMs   = 60000;     // ENS160 necesita ~60s reales
    c.retryMs    = 5000;
    c.errorCount = 0;
    c.consecutiveErrors = 0;

    // RESET
    if (!i2cWrite8(addr, ENS160_REG_OPMODE, ENS160_OPMODE_RESET)) {
        c.state = ChipState::ERROR;
        xSemaphoreGive(i2cMutex);
        return false;
    }

    vTaskDelay(pdMS_TO_TICKS(5));

    uint8_t mode =
        (options & ENS160_OPT_IDLE)
            ? ENS160_OPMODE_IDLE
            : ENS160_OPMODE_STD;

    if (!i2cWrite8(addr, ENS160_REG_OPMODE, mode)) {
        c.state = ChipState::ERROR;
        xSemaphoreGive(i2cMutex);
        return false;
    }

    c.state = ChipState::WARMUP;
    xSemaphoreGive(i2cMutex);
    return true;
}

// ==============================
// READ ALL
// ==============================
static bool ens160ReadAll(uint8_t addr, Ens160Cache& c)
{
    uint16_t v;

    if (!i2cRead16(addr, ENS160_REG_DATA_AQI, v))  return false;
    c.aqi = v;

    if (!i2cRead16(addr, ENS160_REG_DATA_TVOC, v)) return false;
    c.tvoc = v;

    if (!i2cRead16(addr, ENS160_REG_DATA_ECO2, v)) return false;
    c.eco2 = v;

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ==============================
// API PRINCIPAL
// ==============================
bool leerSignalENS160(const Signal& s, float& out)
{
    ensureMutex();

    uint8_t idx = s.address & 0x07;
    ChipContext& cc = ctx[idx];
    Ens160Cache& ec = cache[idx];

    uint32_t now = millis();

    // -----------------------------
    // SAFE
    // -----------------------------
    if (cc.state == ChipState::ERROR) {
        if (now - cc.lastReadTs > cc.retryMs) {
            cc.state = ChipState::UNINITIALIZED;
            cc.consecutiveErrors = 0;
        }
        return false;
    }

    // -----------------------------
    // INIT
    // -----------------------------
    if (cc.state == ChipState::UNINITIALIZED) {
        if (!ens160Init(s.address, s.options)) {
            cc.consecutiveErrors++;
            cc.errorCount++;
            if (cc.consecutiveErrors >= 5)
                cc.state = ChipState::ERROR;
            return false;
        }
        return false;
    }

    // -----------------------------
    // WARMUP
    // -----------------------------
    if (cc.state == ChipState::WARMUP) {
        if (now - cc.initTs < cc.warmupMs)
            return false;
        cc.state = ChipState::READY;
    }

    if (cc.state != ChipState::READY)
        return false;

    // -----------------------------
    // CACHE
    // -----------------------------
    if (!ec.valid || now - ec.lastReadMs > ENS160_CACHE_MS)
    {
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
            return false;

        bool ok = ens160ReadAll(s.address, ec);

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

    // -----------------------------
    // CANALES
    // -----------------------------
    switch (s.channel) {
        case 0: out = ec.aqi;  return true;
        case 1: out = ec.tvoc; return true;
        case 2: out = ec.eco2; return true;
        default: return false;
    }
}

// ==============================
// AUTOCOMPENSACIÓN (OPCIONAL)
// ==============================
bool ens160SetEnvironmentalData(uint8_t addr, float temp, float hum)
{
    ensureMutex();

    uint16_t rh  = (uint16_t)(hum * 64.0f);
    uint16_t tK  = (uint16_t)((temp + 273.15f) * 64.0f);

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
        return false;

    Wire.beginTransmission(addr);
    Wire.write(ENS160_REG_RH_IN);
    Wire.write(rh & 0xFF);
    Wire.write(rh >> 8);
    bool okH = (Wire.endTransmission() == 0);

    Wire.beginTransmission(addr);
    Wire.write(ENS160_REG_TEMP_IN);
    Wire.write(tK & 0xFF);
    Wire.write(tK >> 8);
    bool okT = (Wire.endTransmission() == 0);

    xSemaphoreGive(i2cMutex);
    return okH && okT;
}

// ==============================
// RESET
// ==============================
void ens160Reset()
{
    for (uint8_t i = 0; i < ENS160_MAX_CHIPS; i++) {
        cache[i] = {};
        ctx[i]   = {};
    }
}
