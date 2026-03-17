#include "bh1750_driver.h"
#include "i2c/i2c_chip_guard.h"
#include "i2c/i2c_bus.h"
#include "i2c/i2c_chip_registry.h"
#include <Wire.h>

extern SemaphoreHandle_t semI2C;

// ============================================================
// API PRINCIPAL: READ SIGNAL
// ============================================================

bool bh1750ReadSignal(const Signal& s, float& out)
{
    // 1. Obtención del contexto mediante chipId (Desacoplado de la señal)
    ChipContext* cc = i2cGetChipContextById(s.chipId);
    if (!cc || !cc->inUse) return false;

    uint32_t now = millis();
    uint8_t addr = cc->address;

    // 2. 🔐 GUARD CENTRAL (Manejo de Backoff y Errores)
    if (!i2cChipGuardBeforeRead(cc, now))
        return false;

    // Localizamos caché (mapeo de dirección a índice 0-7)
    Bh1750Cache& c = cache[addr & 0x07];

    // 3. 🔒 PROTECCIÓN DE BUS (Semáforo)
    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(100)) == pdTRUE) 
    {
        bool ok = true;

        // ---------------- INITIALIZATION ----------------
        if (cc->state == ChipState::STATE_UNINITIALIZED)
        {
            if (!bh1750InitInternal(addr, cc->options, cc)) {
                i2cChipGuardOnError(cc, ChipMode::INPUT_ONLY);
                ok = false;
            } else {
                ok = false; // Esperar warmup tras init
            }
        }
        // ---------------- WARMUP CHECK ----------------
        else if (cc->state == ChipState::STATE_WARMUP)
        {
            if (now - cc->initTs < cc->warmupMs) ok = false;
            else cc->state = ChipState::STATE_READY;
        }
        // ---------------- CACHE REFRESH ----------------
        else if (!c.valid || (now - c.lastReadMs > 200)) 
        {
            if (!bh1750ReadAll(addr, c)) {
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
        return false; // Bus ocupado por otra tarea crítica
    }

    out = c.lux;
    return true;
}

// ============================================================
// LÓGICA INTERNA (Privada)
// ============================================================

bool bh1750InitInternal(uint8_t addr, uint8_t options, ChipContext* ctx)
{
    if (!i2cWriteRaw(addr, BH1750_POWER_ON)) return false;
    if (!i2cWriteRaw(addr, BH1750_RESET)) return false;

    uint8_t mode;
    switch (options) {
        case 0: mode = BH1750_CONT_H_RES_MODE;  break;
        case 1: mode = BH1750_CONT_H_RES2_MODE; break;
        case 2: mode = BH1750_CONT_L_RES_MODE;  break;
        default: mode = BH1750_CONT_H_RES_MODE; break;
    }

    if (!i2cWriteRaw(addr, mode)) return false;

    ctx->initTs = millis();
    ctx->warmupMs = (mode == BH1750_CONT_L_RES_MODE) ? 30 : 180;
    ctx->state = ChipState::STATE_WARMUP;
    return true;
}

static bool bh1750ReadAll(uint8_t addr, Bh1750Cache& c)
{
    uint8_t buf[2];
    // Lectura directa de 2 bytes (MSB primero)
    if (!i2cReadRaw(addr, buf, 2)) return false;

    uint16_t raw = (buf[0] << 8) | buf[1];
    c.lux = raw / 1.2f; 

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}
// ============================================================
// METADATA
// ============================================================
void bh1750GetMetadata(ChipMetadata& meta)
{
    static const char* modes[] = {
        "High Res (1 lx)",
        "High Res 2 (0.5 lx)",
        "Low Res (4 lx)"
    };

    meta.name = "BH1750 Light Sensor";
    meta.channelCount = 1;
    meta.mode = ChipMode::INPUT_ONLY; // Indispensable para el i2cChipGuard

    // Configuración de Opción 1 (Resolución)
    meta.opt1.label = "Resolution Mode";
    meta.opt1.values = modes;
    meta.opt1.valueCount = 3;
    meta.opt1.defaultIndex = 0;

    // Configuración de Opción 2 (Nula)
    meta.opt2.label = nullptr;
    meta.opt2.values = nullptr;
    meta.opt2.valueCount = 0;
    meta.opt2.defaultIndex = 0;
}

// ============================================================
// RESET
// ============================================================
void bh1750Reset()
{
    // Limpieza total de la caché de lux por dirección
    for (int i = 0; i < 8; i++) {
        cache[i].valid = false;
        cache[i].lux = 0.0f;
        cache[i].lastReadMs = 0;
    }
}

// ============================================================
// DETECTION
// ============================================================
bool bh1750Detect(uint8_t addr)
{
    // Verificación física de presencia en el bus I2C
    // BH1750 suele estar en 0x23 o 0x5C
    return i2cPing(addr);
}