#include "ina219_driver.h"
#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_bus.h"
#include "i2c/i2c_chip_guard.h"
#include "i2c/i2c_chip_registry.h"
#include <Arduino.h>
#include <Wire.h>

extern SemaphoreHandle_t semI2C;

// ============================================================
// API PRINCIPAL: READ SIGNAL
// ============================================================

bool ina219ReadSignal(const Signal& s, float& out)
{
    // 1. Obtención del contexto mediante chipId (Desacoplado)
    ChipContext* cc = i2cGetChipContextById(s.chipId);
    if (!cc || !cc->inUse) return false;

    uint32_t now = millis();
    uint8_t addr = cc->address;

    // 2. 🔐 GUARD CENTRAL (Manejo de Backoff y reintentos)
    if (!i2cChipGuardBeforeRead(cc, now))
        return false;

    // Localizamos caché (mapeo de dirección a índice 0-7)
    Ina219Cache& c = cache[addr & 0x07];

    // 3. 🔒 PROTECCIÓN DE BUS (Semáforo)
    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(150)) == pdTRUE) 
    {
        bool ok = true;

        // ---------------- INITIALIZATION ----------------
        if (cc->state == ChipState::STATE_UNINITIALIZED)
        {
            if (!ina219InitInternal(addr, cc->options, cc)) {
                i2cChipGuardOnError(cc, ChipMode::INPUT_ONLY);
                ok = false;
            } else {
                ok = false; // Esperar siguiente ciclo tras init
            }
        }
        // ---------------- CACHE REFRESH ----------------
        else if (!c.valid || (now - c.lastReadMs > 200)) // Ventana de 200ms
        {
            if (!ina219ReadAll(addr, c)) {
                i2cChipGuardOnError(cc, ChipMode::INPUT_ONLY);
                ok = false;
            } else {
                i2cChipGuardOnSuccess(cc, now);
            }
        }

        xSemaphoreGive(semI2C);
        if (!ok) return false;
    } 
    else {
        return false; // Bus ocupado
    }

    // 4. RETORNO DE CANAL ESPECÍFICO
    switch (s.channel)
    {
        case 0: out = c.voltage; return true;
        case 1: out = c.current; return true;
        case 2: out = c.power;   return true;
        default: return false;
    }
}

// ============================================================
// LÓGICA INTERNA (Privada)
// ============================================================

bool ina219InitInternal(uint8_t addr, uint8_t options, ChipContext* ctx)
{
    uint16_t config;

    // Configuración según opciones: bit 13 define bus range (0=16V, 1=32V)
    if (options == 1)
        config = 0x399F;  // Range 32V, Gain /8, 12-bit res
    else
        config = 0x019F;  // Range 16V, Gain /8, 12-bit res

    if (!i2cWrite16(addr, INA219_REG_CONFIG, config))
        return false;

    ctx->state = ChipState::STATE_READY;
    ctx->warmupMs = 5; // Estabilización mínima
    ctx->initTs = millis();
    return true;
}

static bool ina219ReadAll(uint8_t addr, Ina219Cache& c)
{
    uint16_t rawBus;
    uint16_t rawShunt;

    // Leemos voltaje de bus y caída en shunt
    if (!i2cRead16(addr, INA219_REG_BUS_VOLT, rawBus)) return false;
    if (!i2cRead16(addr, INA219_REG_SHUNT_VOLT, rawShunt)) return false;

    // Procesamiento de Voltaje (Bits 3-15 son el valor)
    rawBus >>= 3;
    c.voltage = (float)rawBus * INA219_BUS_LSB;

    // Procesamiento de Corriente (Ley de Ohm: I = Vshunt / Rshunt)
    int16_t shunt = (int16_t)rawShunt;
    float shuntV = (float)shunt * INA219_SHUNT_LSB;

    c.current = shuntV / INA219_SHUNT_OHMS;
    c.power   = c.voltage * c.current;

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ============================================================
// METADATA, RESET Y DETECT
// ============================================================

void ina219GetMetadata(ChipMetadata& meta)
{
    static const char* ranges[] = { "16V Range", "32V Range" };

    meta.name = "INA219 Power Monitor";
    meta.channelCount = 3;
    meta.mode = ChipMode::INPUT_ONLY;

    meta.opt1 = {"Voltage Range", ranges, 2, 0};
    meta.opt2 = {nullptr, nullptr, 0, 0};
}

void ina219Reset()
{
    for (int i = 0; i < 8; i++) cache[i] = {};
}

bool ina219Detect(uint8_t addr)
{
    uint16_t config;
    // Si podemos leer el registro de configuración, el chip está presente
    return i2cRead16(addr, INA219_REG_CONFIG, config);
}