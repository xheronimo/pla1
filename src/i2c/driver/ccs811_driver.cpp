#include "ccs811_driver.h"
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

bool ccs811ReadSignal(const Signal& s, float& out)
{
    // 1. Obtención del contexto mediante chipId
    ChipContext* cc = i2cGetChipContextById(s.chipId);
    if (!cc || !cc->inUse) return false;

    uint32_t now = millis();
    uint8_t addr = cc->address;

    // 2. 🔐 GUARD CENTRAL
    if (!i2cChipGuardBeforeRead(cc, now))
        return false;

    // Localizamos caché (mapeo de dirección a índice 0-7)
    Ccs811Cache& c = cache[addr & 0x07];

    // 3. 🔒 PROTECCIÓN DE BUS (Semáforo)
    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(150)) == pdTRUE) 
    {
        bool ok = true;

        // ---------------- INITIALIZATION ----------------
        if (cc->state == ChipState::STATE_UNINITIALIZED)
        {
            if (!ccs811InitInternal(addr, cc->options, cc)) {
                i2cChipGuardOnError(cc, ChipMode::INPUT_ONLY);
                ok = false;
            } else {
                ok = false; // Esperar siguiente ciclo tras init
            }
        }
        // ---------------- WARMUP CHECK ----------------
        else if (cc->state == ChipState::STATE_WARMUP)
        {
            if (now - cc->initTs < cc->warmupMs) ok = false;
            else cc->state = ChipState::STATE_READY;
        }
        // ---------------- CACHE REFRESH ----------------
        else if (!c.valid || (now - c.lastReadMs > CCS811_CACHE_MS))
        {
            if (!ccs811ReadAll(addr, c)) {
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

    // 4. RETORNO DE CANAL
    switch (s.channel)
    {
        case 0: out = (float)c.eco2; return true;
        case 1: out = (float)c.tvoc; return true;
        default: return false;
    }
}

// ============================================================
// LÓGICA INTERNA (Privada)
// ============================================================

bool ccs811InitInternal(uint8_t addr, uint8_t options, ChipContext* ctx)
{
    // Software reset (Secuencia específica del datasheet)
    uint8_t resetSeq[4] = {0x11, 0xE5, 0x72, 0x8A};
    if (!i2cWriteBytes(addr, CCS811_REG_SW_RESET, resetSeq, 4)) return false;

    delay(20); // Tiempo para el reset

    // APP_START: Escribir en este registro sin datos para pasar de Boot a App
    Wire.beginTransmission(addr);
    Wire.write(CCS811_REG_APP_START);
    if (Wire.endTransmission() != 0) return false;

    delay(20);

    // Measurement mode: Definir la tasa de muestreo
    uint8_t mode;
    switch (options)
    {
        case 0: mode = 0x10; break; // 1s
        case 1: mode = 0x20; break; // 10s
        case 2: mode = 0x30; break; // 60s
        default: mode = 0x10; break;
    }

    if (!i2cWrite8(addr, CCS811_REG_MEAS_MODE, mode)) return false;

    ctx->initTs = millis();
    ctx->warmupMs = 20000; // El CCS811 necesita 20 segundos para estabilizar el calentador
    ctx->state = ChipState::STATE_WARMUP;
    return true;
}

static bool ccs811ReadAll(uint8_t addr, Ccs811Cache& c)
{
    uint8_t buf[4];
    // Leemos ráfaga de 4 bytes (eCO2 y TVOC)
    if (!i2cReadBytes(addr, CCS811_REG_ALG_RESULT, buf, 4))
        return false;

    c.eco2 = (buf[0] << 8) | buf[1];
    c.tvoc = (buf[2] << 8) | buf[3];

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ============================================================
// METADATA, RESET Y DETECT
// ============================================================

void ccs811GetMetadata(ChipMetadata& meta)
{
    static const char* rates[] = { "1s", "10s", "60s" };

    meta.name = "CCS811 Air Quality";
    meta.channelCount = 2;
    meta.mode = ChipMode::INPUT_ONLY;

    meta.opt1 = {"Sample Rate", rates, 3, 0};
    meta.opt2 = {nullptr, nullptr, 0, 0};
}

void ccs811Reset()
{
    for (int i = 0; i < 8; i++) cache[i] = {};
}

bool ccs811Detect(uint8_t addr)
{
    uint8_t status;
    // Un simple ping al registro de status para ver si responde
    return i2cRead8(addr, CCS811_REG_STATUS, status);
}