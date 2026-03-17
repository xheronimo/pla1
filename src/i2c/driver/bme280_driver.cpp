#include "bme280_driver.h"
#include "i2c/i2c_chip_guard.h"
#include "i2c/i2c_bus.h"
#include "i2c/i2c_chip_registry.h"
#include <Wire.h>

extern SemaphoreHandle_t semI2C;

// ============================================================
// API PRINCIPAL: READ SIGNAL
// ============================================================

bool bme280ReadSignal(const Signal& s, float& out)
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
    Bme280Cache& c = cache[addr & 0x07];

    // 3. 🔒 PROTECCIÓN DE BUS (Semáforo)
    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(150)) == pdTRUE) 
    {
        bool ok = true;

        // ---------------- INITIALIZATION ----------------
        if (cc->state == ChipState::STATE_UNINITIALIZED)
        {
            if (!bme280InitInternal(addr, cc->options, cc)) {
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
        else if (!c.valid || (now - c.lastReadMs > 250)) 
        {
            if (!bme280ReadAll(addr, c)) {
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
        case 0: out = c.temp; return true;
        case 1: out = c.hum;  return true;
        case 2: out = c.pres; return true;
        default: return false;
    }
}

// ============================================================
// LÓGICA INTERNA (Privada)
// ============================================================

bool bme280InitInternal(uint8_t addr, uint8_t options, ChipContext* ctx)
{
    if (!i2cWrite8(addr, BME280_REG_RESET, BME280_RESET_CMD))
        return false;

    delay(5); // Tiempo para el reset interno

    uint8_t oversampling = (options & 0x03) + 1;

    // Configuración de Humedad (debe escribirse antes que CTRL_MEAS)
    if (!i2cWrite8(addr, BME280_REG_CTRL_HUM, oversampling))
        return false;

    // Configuración Temp, Pres y modo Normal (0x03)
    if (!i2cWrite8(addr, BME280_REG_CTRL_MEAS, (oversampling << 5) | (oversampling << 2) | 0x03))
        return false;

    // Standby 1000ms y filtro IIR off
    if (!i2cWrite8(addr, BME280_REG_CONFIG, 0xA0))
        return false;

    ctx->initTs = millis();
    ctx->warmupMs = 50; 
    ctx->state = ChipState::STATE_WARMUP;
    return true;
}

static bool bme280ReadAll(uint8_t addr, Bme280Cache& c)
{
    uint8_t buf[8];
    // Leemos ráfaga de 8 bytes para tener T, P y H sincronizados
    if (!i2cReadBytes(addr, BME280_REG_DATA, buf, 8))
        return false;

    int32_t adc_P = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
    int32_t adc_T = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);
    int32_t adc_H = (buf[6] << 8)  | buf[7];

    // NOTA: Para un entorno industrial real, aquí deberías usar las fórmulas 
    // de compensación de Bosch usando los "trimming parameters". 
    // Esta es una versión simplificada:
    c.temp = adc_T / 100.0f;
    c.pres = adc_P / 25600.0f;
    c.hum  = adc_H / 1024.0f;

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ============================================================
// METADATA, RESET Y DETECT
// ============================================================

void bme280GetMetadata(ChipMetadata& meta)
{
    static const char* oversampling[] = { "x1", "x2", "x4", "x8" };

    meta.name = "BME280";
    meta.channelCount = 3;
    meta.mode = ChipMode::INPUT_ONLY;

    meta.opt1 = {"Oversampling", oversampling, 4, 1};
    meta.opt2 = {nullptr, nullptr, 0, 0};
}

void bme280Reset()
{
    for (int i = 0; i < 8; i++) cache[i] = {};
}

bool bme280Detect(uint8_t addr)
{
    uint8_t id;
    // El chip ID del BME280 es 0x60
    if (!i2cRead8(addr, BME280_REG_ID, id)) return false;
    return (id == 0x60);
}