#include "i2c/ccs811_driver.h"
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// ==============================
// REGISTROS CCS811
// ==============================
#define CCS811_REG_STATUS      0x00
#define CCS811_REG_MEAS_MODE   0x01
#define CCS811_REG_ALG_RESULT  0x02
#define CCS811_REG_HW_ID       0x20
#define CCS811_REG_APP_START   0xF4
#define CCS811_REG_SW_RESET    0xFF

#define CCS811_HW_ID           0x81

// ==============================
// CACHE
// ==============================
struct Ccs811Cache {
    bool valid = false;
    uint16_t eco2 = 0;
    uint16_t tvoc = 0;
    uint32_t lastReadMs = 0;
};

static Ccs811Cache cache[CCS811_MAX_CHIPS];
static ChipContext ctx[CCS811_MAX_CHIPS];

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
static bool i2cWrite(uint8_t addr, const uint8_t* data, uint8_t len)
{
    Wire.beginTransmission(addr);
    Wire.write(data, len);
    return Wire.endTransmission() == 0;
}

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

// ==============================
// INIT
// ==============================
bool ccs811Init(uint8_t addr, uint8_t driveMode)
{
    ensureMutex();

    uint8_t idx = addr & 0x07;
    ChipContext& c = ctx[idx];

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
        return false;

    c.state      = ChipState::INITIALIZING;
    c.initTs     = millis();
    c.warmupMs   = 120000;   // ⚠️ CCS811 necesita hasta 2 minutos reales
    c.retryMs    = 5000;
    c.errorCount = 0;

    // -------------------------
    // HW ID CHECK
    // -------------------------
    uint8_t hwid = 0;
    if (!i2cRead(addr, CCS811_REG_HW_ID, &hwid, 1) || hwid != CCS811_HW_ID)
    {
        c.state = ChipState::ERROR;
        xSemaphoreGive(i2cMutex);
        return false;
    }

    // -------------------------
    // APP START
    // -------------------------
    uint8_t cmd = CCS811_REG_APP_START;
    if (!i2cWrite(addr, &cmd, 1))
    {
        c.state = ChipState::ERROR;
        xSemaphoreGive(i2cMutex);
        return false;
    }

    vTaskDelay(pdMS_TO_TICKS(2));

    // -------------------------
    // MEAS MODE
    // driveMode:
    // 0 → idle
    // 1 → 1s
    // 2 → 10s
    // 3 → 60s
    // 4 → 250ms
    // -------------------------
    uint8_t meas[2];
    meas[0] = CCS811_REG_MEAS_MODE;
    meas[1] = (driveMode & 0x07) << 4;

    if (!i2cWrite(addr, meas, 2))
    {
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
static bool ccs811ReadAll(uint8_t addr, Ccs811Cache& c)
{
    uint8_t buf[8];
    if (!i2cRead(addr, CCS811_REG_ALG_RESULT, buf, 8))
        return false;

    c.eco2 = (buf[0] << 8) | buf[1];
    c.tvoc = (buf[2] << 8) | buf[3];

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ==============================
// API PRINCIPAL
// ==============================
bool leerSignalCCS811(const Signal& s, float& out)
{
    ensureMutex();

    uint8_t idx = s.address & 0x07;
    ChipContext& cc = ctx[idx];
    Ccs811Cache& sc = cache[idx];

    uint32_t now = millis();

    // -----------------------------
    // ESTADOS
    // -----------------------------
    if (cc.state == ChipState::UNINITIALIZED)
    {
        ccs811Init(s.address);
        return false;
    }

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
    if (!sc.valid || now - sc.lastReadMs > CCS811_CACHE_MS)
    {
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE)
            return false;

        bool ok = ccs811ReadAll(s.address, sc);

        xSemaphoreGive(i2cMutex);

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
        case 0: out = sc.eco2; return true;
        case 1: out = sc.tvoc; return true;
        default: return false;
    }
}

// ==============================
// RESET
// ==============================
void ccs811Reset()
{
    for (uint8_t i = 0; i < CCS811_MAX_CHIPS; i++)
    {
        cache[i] = {};
        ctx[i]   = {};
    }
}
