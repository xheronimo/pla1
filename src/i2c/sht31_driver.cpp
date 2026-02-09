#include "i2c/sht31_driver.h"
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// =================================================
// COMANDOS
// =================================================
#define SHT31_CMD_SOFT_RESET        0x30A2
#define SHT31_CMD_HEATER_ENABLE    0x306D
#define SHT31_CMD_HEATER_DISABLE   0x3066

#define CMD_MEAS_HIGH  0x2400
#define CMD_MEAS_MED   0x240B
#define CMD_MEAS_LOW   0x2416

// =================================================
// CACHE
// =================================================
struct Sht31Cache {
    bool     valid = false;
    float    temperature = 0.0f;
    float    humidity    = 0.0f;
    uint32_t lastReadMs  = 0;
};

static Sht31Cache  cache[SHT31_MAX_CHIPS];
static ChipContext ctx[SHT31_MAX_CHIPS];

// =================================================
// MUTEX I2C (GLOBAL)
// =================================================
static SemaphoreHandle_t i2cMutex = nullptr;

static void ensureMutex()
{
    if (!i2cMutex)
        i2cMutex = xSemaphoreCreateMutex();
}

// =================================================
// LOW LEVEL
// =================================================
static bool sht31WriteCmd(uint8_t addr, uint16_t cmd)
{
    Wire.beginTransmission(addr);
    Wire.write(cmd >> 8);
    Wire.write(cmd & 0xFF);
    return Wire.endTransmission() == 0;
}

static bool sht31Read(uint8_t addr, uint8_t* buf, uint8_t len)
{
    if (Wire.requestFrom(addr, len) != len)
        return false;

    for (uint8_t i = 0; i < len; i++)
        buf[i] = Wire.read();

    return true;
}

static uint16_t measureCmd(Sht31Repeatability r)
{
    switch (r) {
        case Sht31Repeatability::LOW:    return CMD_MEAS_LOW;
        case Sht31Repeatability::MEDIUM: return CMD_MEAS_MED;
        default:                         return CMD_MEAS_HIGH;
    }
}

// =================================================
// INIT
// =================================================
bool sht31Init(uint8_t addr, const Sht31Options& opt)
{
    ensureMutex();

    uint8_t idx = addr & 0x07;
    ChipContext& c = ctx[idx];

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
        return false;

    c.state     = ChipState::INITIALIZING;
    c.initTs    = millis();
    c.warmupMs  = 15;
    c.retryMs   = 1000;
    c.errorCount = 0;

    if (!sht31WriteCmd(addr, SHT31_CMD_SOFT_RESET))
        goto fail;

    vTaskDelay(pdMS_TO_TICKS(2));

    if (opt.heater) {
        if (!sht31WriteCmd(addr, SHT31_CMD_HEATER_ENABLE))
            goto fail;
    } else {
        sht31WriteCmd(addr, SHT31_CMD_HEATER_DISABLE);
    }

    c.state = ChipState::WARMUP;
    xSemaphoreGive(i2cMutex);
    return true;

fail:
    c.state = ChipState::ERROR;
    xSemaphoreGive(i2cMutex);
    return false;
}

// =================================================
// READ ALL
// =================================================
static bool sht31ReadAll(uint8_t addr, Sht31Cache& c, Sht31Repeatability rep)
{
    uint16_t cmd = measureCmd(rep);

    if (!sht31WriteCmd(addr, cmd))
        return false;

    vTaskDelay(pdMS_TO_TICKS(15));

    uint8_t buf[6];
    if (!sht31Read(addr, buf, 6))
        return false;

    uint16_t rawT = (buf[0] << 8) | buf[1];
    uint16_t rawH = (buf[3] << 8) | buf[4];

    c.temperature = -45.0f + 175.0f * (rawT / 65535.0f);
    c.humidity    = 100.0f * (rawH / 65535.0f);

    c.valid      = true;
    c.lastReadMs = millis();
    return true;
}

// =================================================
// API PRINCIPAL
// =================================================
bool leerSignalSHT31(const Signal& s, float& out)
{
    uint8_t idx = s.address & 0x07;
    ChipContext& cc = ctx[idx];
    Sht31Cache&  sc = cache[idx];

    uint32_t now = millis();

    // -----------------------------
    // ESTADOS
    // -----------------------------
    if (cc.state == ChipState::UNINITIALIZED)
        return false;

    if (cc.state == ChipState::WARMUP)
    {
        if (now - cc.initTs < cc.warmupMs)
            return false;

        cc.state = ChipState::READY;
    }

    if (cc.state != ChipState::READY)
        return false;

    // -----------------------------
    // CACHE
    // -----------------------------
    if (!sc.valid || now - sc.lastReadMs > SHT31_CACHE_MS)
    {
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
            return false;

        bool ok = sht31ReadAll(
            s.address,
            sc,
            (Sht31Repeatability)s.options
        );

        xSemaphoreGive(i2cMutex);   // ✅ SIEMPRE se libera aquí

        if (!ok) {
    cc.consecutiveErrors++;
    cc.errorCount++;

    if (cc.consecutiveErrors >= 5) {
        cc.state = ChipState::ERROR;   // 🔴 CHIP SAFE
    }
    return false;
}

    }

    // -----------------------------
    // CANALES
    // -----------------------------
    switch (s.channel)
    {
        case 0: out = sc.temperature; return true;
        case 1: out = sc.humidity;    return true;
        default: return false;
    }
}


// =================================================
// RESET
// =================================================
void sht31Reset()
{
    for (uint8_t i = 0; i < SHT31_MAX_CHIPS; i++) {
        cache[i] = {};
        ctx[i]   = {};
    }
}
