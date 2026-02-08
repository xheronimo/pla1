#include "i2c/sht31_driver.h"
#include <Wire.h>

// ==============================
// COMANDOS SHT31
// ==============================
#define SHT31_CMD_MEASURE_HIGHREP  0x2400
#define SHT31_CMD_SOFT_RESET       0x30A2

// ==============================
// CACHE POR CHIP
// ==============================
struct Sht31Cache {
    bool valid = false;
    float temperature = 0.0f;
    float humidity    = 0.0f;
    uint32_t lastReadMs = 0;
};

static Sht31Cache   cache[SHT31_MAX_CHIPS];
static ChipContext  ctx[SHT31_MAX_CHIPS];

// ==============================
// LOW LEVEL I2C
// ==============================
static bool sht31WriteCmd(uint8_t addr, uint16_t cmd)
{
    Wire.beginTransmission(addr);
    Wire.write(cmd >> 8);
    Wire.write(cmd & 0xFF);
    return Wire.endTransmission() == 0;
}

static bool sht31ReadData(uint8_t addr, uint8_t* buf, uint8_t len)
{
    if (Wire.requestFrom(addr, len) != len)
        return false;

    for (uint8_t i = 0; i < len; i++)
        buf[i] = Wire.read();

    return true;
}

// ==============================
// INIT

/*// ==============================
bool sht31Init(uint8_t addr)
{
    uint8_t idx = addr & 0x07;
    ChipContext& c = ctx[idx];

    c.state     = ChipState::INITIALIZING;
    c.initTs    = millis();
    c.warmupMs  = 15;        // datasheet ~15 ms
    c.retryMs   = 1000;
    c.errorCount = 0;

    // Soft reset
    if (!sht31WriteCmd(addr, SHT31_CMD_SOFT_RESET))
    {
        c.state = ChipState::ERROR;
        return false;
    }

    c.state = ChipState::WARMUP;
    return true;
}
    */


// ==============================
// INIT con selección de comando
// ==============================
bool sht31Init(uint8_t addr, uint8_t mode)
{
    uint8_t idx = addr & 0x07;
    ChipContext& c = ctx[idx];

    c.state     = ChipState::INITIALIZING;
    c.initTs    = millis();
    c.warmupMs  = 15;        // Tiempo para que el silicio se estabilice
    c.retryMs   = 1000;

    // 1. Primero un Soft Reset para limpiar el estado del sensor
    if (!sht31WriteCmd(addr, SHT31_CMD_SOFT_RESET))
    {
        c.state = ChipState::ERROR;
        return false;
    }

    vTaskDelay(pdMS_TO_TICKS(2)); // Breve espera tras reset

    // 2. Si el modo es 1, forzamos una lectura de alta repetibilidad 
    // o configuramos el comportamiento del chip
    if (mode == 1) 
    {
        // Enviamos el comando 0x2400 que indicaste
        if (!sht31WriteCmd(addr, 0x2400)) // SHT31_CMD_MEASURE_HIGHREP
        {
            c.state = ChipState::ERROR;
            return false;
        }
    }

    c.state = ChipState::WARMUP;
    return true;
}

// ==============================
// READ ALL (TEMP + HUM)
// ==============================
static bool sht31ReadAll(uint8_t addr, Sht31Cache& c)
{
    if (!sht31WriteCmd(addr, SHT31_CMD_MEASURE_HIGHREP))
        return false;

    vTaskDelay(pdMS_TO_TICKS(15)); // NO bloquea

    uint8_t buf[6];
    if (!sht31ReadData(addr, buf, 6))
        return false;

    uint16_t rawT = (buf[0] << 8) | buf[1];
    uint16_t rawH = (buf[3] << 8) | buf[4];

    // Conversión datasheet
    c.temperature = -45.0f + 175.0f * (rawT / 65535.0f);
    c.humidity    = 100.0f * (rawH / 65535.0f);

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ==============================
// API PRINCIPAL
// ==============================
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
    {
        sht31Init(s.address);
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
    if (!sc.valid || now - sc.lastReadMs > SHT31_CACHE_MS)
    {
        if (!sht31ReadAll(s.address, sc))
        {
            cc.errorCount++;
            if (cc.errorCount > 5)
                cc.state = ChipState::ERROR;
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

// ==============================
// RESET
// ==============================
void sht31Reset()
{
    for (uint8_t i = 0; i < SHT31_MAX_CHIPS; i++)
    {
        cache[i] = {};
        ctx[i]   = {};
    }
}
