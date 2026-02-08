#include "i2c/ina219_driver.h"
#include <Wire.h>

// ==============================
// REGISTROS INA219
// ==============================
#define INA219_REG_CONFIG       0x00
#define INA219_REG_SHUNT_VOLT   0x01
#define INA219_REG_BUS_VOLT     0x02

// ==============================
// CONFIGURACIÓN HW
// ==============================
constexpr float INA219_SHUNT_OHMS = 0.1f;     // ajustar a tu placa
constexpr float INA219_BUS_LSB    = 0.004f;   // 4 mV
constexpr float INA219_SHUNT_LSB  = 0.00001f; // 10 µV

// ==============================
// CACHE
// ==============================
struct Ina219Cache {
    bool     valid = false;
    float    voltage = 0.0f;
    float    current = 0.0f;
    float    power   = 0.0f;
    uint32_t lastReadMs = 0;
};

static Ina219Cache  cache[INA219_MAX_CHIPS];
static ChipContext  ctx[INA219_MAX_CHIPS];

// ==============================
// LOW LEVEL I2C
// ==============================
static bool i2cRead16(uint8_t addr, uint8_t reg, uint16_t& out)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(addr, (uint8_t)2) != 2)
        return false;

    out = (Wire.read() << 8) | Wire.read();
    return true;
}

// ==============================
// INIT (una vez)
// ==============================
static bool ina219Init(uint8_t addr, ChipContext& c, uint8_t mode)
{
    Wire.beginTransmission(addr);
    Wire.write(INA219_REG_CONFIG);
    if (mode == 1) {
            // Modo 1: Rango 32V (para picos) y máximo filtrado
            Wire.write(0x39); // 32V, 128 samples
            Wire.write(0xFF); 
        } else {
            // Modo 0 (Default): 16V range, 128 samples (estable)
            Wire.write(0x01); 
            Wire.write(0x9F); 
        }
    if (Wire.endTransmission() != 0)
        return false;

    c.state = ChipState::READY;
    return true;
}

// ==============================
// READ ALL
// ==============================
static bool ina219ReadAll(uint8_t addr, Ina219Cache& c)
{
    uint16_t rawBus   = 0;
    uint16_t rawShunt = 0;

    if (!i2cRead16(addr, INA219_REG_BUS_VOLT, rawBus))
        return false;

    if (!i2cRead16(addr, INA219_REG_SHUNT_VOLT, rawShunt))
        return false;

    rawBus >>= 3;
    c.voltage = rawBus * INA219_BUS_LSB;

    int16_t shunt = (int16_t)rawShunt;
    float shuntV = shunt * INA219_SHUNT_LSB;

    c.current = shuntV / INA219_SHUNT_OHMS;
    c.power   = c.voltage * c.current;

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ==============================
// API PRINCIPAL
// ==============================
bool leerSignalINA219(const Signal& s, float& out)
{
    uint8_t idx = s.address & 0x07;
    Ina219Cache& c = cache[idx];
    ChipContext& cc = ctx[idx];

    uint32_t now = millis();

    // INIT NO BLOQUEANTE
    if (cc.state == ChipState::UNINITIALIZED)
    {
        if (!ina219Init(s.address, cc))
        {
            cc.errorCount++;
            cc.state = ChipState::ERROR;
            return false;
        }
    }

    // CACHE
    if (!c.valid || now - c.lastReadMs > INA219_CACHE_MS)
    {
        if (!ina219ReadAll(s.address, c))
        {
            cc.errorCount++;
            return false;
        }
    }

    switch (s.channel)
    {
        case 0: out = c.voltage; return true;
        case 1: out = c.current; return true;
        case 2: out = c.power;   return true;
        default: return false;
    }
}

// ==============================
// RESET
// ==============================
void ina219Reset()
{
    for (uint8_t i = 0; i < INA219_MAX_CHIPS; i++)
    {
        cache[i] = {};
        ctx[i]   = {};
    }
}
