#include "ens160_driver.h"
#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_chip_guard.h"
#include "i2c/i2c_bus.h"
#include "i2c/i2c_chip_registry.h"

#include <Arduino.h>
#include <Wire.h>

extern SemaphoreHandle_t semI2C;

// ============================================================
// API PRINCIPAL: READ SIGNAL
// ============================================================

bool ens160ReadSignal(const Signal& s, float& out)
{
    // 1. Obtener contexto por chipId (Desacoplado)
    ChipContext* cc = i2cGetChipContextById(s.chipId);
    if (!cc || !cc->inUse) return false;

    uint32_t now = millis();
    uint8_t addr = cc->address;

    // 2. 🔐 GUARD CENTRAL (Manejo de estados y reintentos)
    if (!i2cChipGuardBeforeRead(cc, now))
        return false;

    // Localizamos caché (máximo 8 sensores por bus)
    Ens160Cache& c = cache[addr & 0x07];

    // 3. 🔒 PROTECCIÓN DE BUS (Semáforo)
    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(150)) == pdTRUE) 
    {
        bool ok = true;

        // ---------------- INITIALIZATION ----------------
        if (cc->state == ChipState::STATE_UNINITIALIZED)
        {
            if (!ens160InitInternal(addr, cc->options, cc)) {
                i2cChipGuardOnError(cc, ChipMode::INPUT_ONLY);
                ok = false;
            } else {
                ok = false; // Esperar siguiente ciclo tras enviar comandos
            }
        }
        // ---------------- WARMUP CHECK ----------------
        else if (cc->state == ChipState::STATE_WARMUP)
        {
            // El ENS160 necesita 60s para que la placa MOX se estabilice
            if (now - cc->initTs < cc->warmupMs) ok = false;
            else cc->state = ChipState::STATE_READY;
        }
        // ---------------- CACHE REFRESH ----------------
        else if (!c.valid || (now - c.lastReadMs > 500)) 
        {
            if (!ens160ReadAll(addr, c)) {
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
        return false; // Bus ocupado (ej: escaneo Web)
    }

    // 4. RETORNO DE VALORES
    switch (s.channel)
    {
        case 0: out = (float)c.aqi;  return true;
        case 1: out = (float)c.tvoc; return true;
        case 2: out = (float)c.eco2; return true;
        default: return false;
    }
}

// ============================================================
// LÓGICA INTERNA (Privada)
// ============================================================

bool ens160InitInternal(uint8_t addr, uint8_t options, ChipContext* ctx)
{
    // RESET del sensor
    if (!i2cWrite8(addr, ENS160_REG_OPMODE, ENS160_OPMODE_RESET))
        return false;

    delay(20); // Tiempo para el reinicio

    // Selección de modo (Standard por defecto)
    uint8_t mode = (options == 1) ? ENS160_OPMODE_IDLE : ENS160_OPMODE_STD;

    if (!i2cWrite8(addr, ENS160_REG_OPMODE, mode))
        return false;

    ctx->initTs = millis();
    ctx->warmupMs = 60000; // 60 segundos de precalentamiento industrial
    ctx->state = ChipState::STATE_WARMUP;
    return true;
}

static bool ens160ReadAll(uint8_t addr, Ens160Cache& c)
{
    uint16_t v;
    // Lectura de los 3 parámetros de calidad
    if (!i2cRead16(addr, ENS160_REG_DATA_AQI, v)) return false;
    c.aqi = v;

    if (!i2cRead16(addr, ENS160_REG_DATA_TVOC, v)) return false;
    c.tvoc = v;

    if (!i2cRead16(addr, ENS160_REG_DATA_ECO2, v)) return false;
    c.eco2 = v;

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ============================================================
// COMPENSACIÓN AMBIENTAL (Invocable desde fuera del driver)
// ============================================================

bool ens160SetEnvironmentalData(uint8_t addr, float temp, float hum)
{
    uint16_t rh = (uint16_t)(hum * 64.0f);
    uint16_t t  = (uint16_t)((temp + 273.15f) * 64.0f);

    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(100)) == pdTRUE) {
        bool ok = i2cWrite16(addr, ENS160_REG_RH_IN, rh) && 
                  i2cWrite16(addr, ENS160_REG_TEMP_IN, t);
        xSemaphoreGive(semI2C);
        return ok;
    }
    return false;
}

// ============================================================
// METADATA, RESET Y DETECT
// ============================================================

void ens160GetMetadata(ChipMetadata& meta)
{
    static const char* modes[] = { "Standard Mode", "Idle Mode" };

    meta.name = "ENS160 Air Quality";
    meta.channelCount = 3;
    meta.mode = ChipMode::INPUT_ONLY;

    meta.opt1 = {"Operating Mode", modes, 2, 0};
    meta.opt2 = {nullptr, nullptr, 0, 0};
}

void ens160Reset()
{
    for (int i = 0; i < 8; i++) cache[i] = {};
}

bool ens160Detect(uint8_t addr)
{
    uint16_t id;
    // ID del ENS160 es 0x0160
    if (!i2cRead16(addr, 0x00, id)) return false;
    return (id == 0x0160);
}