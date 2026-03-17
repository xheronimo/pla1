#include "sht31_driver.h"
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

bool sht31ReadSignal(const Signal& s, float& out)
{
    // 1. Obtención del contexto mediante chipId
    ChipContext* cc = i2cGetChipContextById(s.chipId);
    if (!cc || !cc->inUse) return false;

    uint32_t now = millis();
    uint8_t addr = cc->address;

    // 2. 🔐 GUARD CENTRAL (Manejo de Backoff y reintentos)
    if (!i2cChipGuardBeforeRead(cc, now))
        return false;

    // Localizamos caché (mapeo de dirección a índice 0-7)
    Sht31Cache& c = cache[addr & 0x07];

    // 3. 🔒 PROTECCIÓN DE BUS (Semáforo)
    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(150)) == pdTRUE) 
    {
        bool ok = true;

        // ---------------- INITIALIZATION ----------------
        if (cc->state == ChipState::STATE_UNINITIALIZED)
        {
            if (!sht31InitInternal(addr, cc)) {
                i2cChipGuardOnError(cc, ChipMode::INPUT_ONLY);
                ok = false;
            } else {
                ok = false; // Esperar warmup tras reset
            }
        }
        // ---------------- WARMUP CHECK ----------------
        else if (cc->state == ChipState::STATE_WARMUP)
        {
            if (now - cc->initTs < cc->warmupMs) ok = false;
            else cc->state = ChipState::STATE_READY;
        }
        // ---------------- CACHE REFRESH ----------------
        else if (!c.valid || (now - c.lastReadMs > 500)) 
        {
            if (!sht31ReadAll(addr, c, cc->options)) {
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

    // 4. RETORNO DE CANAL (0: Temp, 1: Hum)
    if (s.channel == 0) {
        out = c.temperature;
        return true;
    } else if (s.channel == 1) {
        out = c.humidity;
        return true;
    }
    
    return false;
}

// ============================================================
// LÓGICA INTERNA (Privada)
// ============================================================

bool sht31InitInternal(uint8_t addr, ChipContext* ctx)
{
    // Soft reset comando 0x30A2
    Wire.beginTransmission(addr);
    Wire.write(0x30);
    Wire.write(0xA2);
    if (Wire.endTransmission() != 0) return false;

    ctx->initTs = millis();
    ctx->warmupMs = 20; // El SHT31 resetea rápido
    ctx->state = ChipState::STATE_WARMUP;
    return true;
}

static bool sht31ReadAll(uint8_t addr, Sht31Cache& c, uint8_t options)
{
    uint16_t cmd;
    switch (options) {
        case 1:  cmd = SHT31_CMD_MEDREP; break;
        case 2:  cmd = SHT31_CMD_LOWREP; break;
        default: cmd = SHT31_CMD_HIGHREP; break;
    }

    // Iniciar medición
    Wire.beginTransmission(addr);
    Wire.write(cmd >> 8);
    Wire.write(cmd & 0xFF);
    if (Wire.endTransmission() != 0) return false;

    // Tiempo de conversión según datasheet (max 15ms para High Rep)
    delay(15); 

    // Leer 6 bytes (T_msb, T_lsb, T_crc, H_msb, H_lsb, H_crc)
    uint8_t buf[6];
    if (Wire.requestFrom(addr, (uint8_t)6) != 6) return false;
    
    for (uint8_t i = 0; i < 6; i++) buf[i] = Wire.read();

    // Nota: Aquí se debería validar el CRC para máxima seguridad industrial
    uint16_t rawT = (buf[0] << 8) | buf[1];
    uint16_t rawH = (buf[3] << 8) | buf[4];

    c.temperature = -45.0f + 175.0f * ((float)rawT / 65535.0f);
    c.humidity    = 100.0f * ((float)rawH / 65535.0f);

    c.valid = true;
    c.lastReadMs = millis();
    return true;
}

// ============================================================
// METADATA, RESET Y DETECT
// ============================================================

void sht31GetMetadata(ChipMetadata& meta)
{
    static const char* reps[] = { "High Precision", "Medium Precision", "Low Precision" };

    memset(&meta, 0, sizeof(ChipMetadata));
    strcpy(meta.name, "SHT31 Temp/Humidity");
    meta.channelCount = 2;
    meta.mode = ChipMode::INPUT_ONLY;

    meta.opt1.label = "Repeatability";
    meta.opt1.values = reps;
    meta.opt1.valueCount = 3;
    meta.opt1.defaultIndex = 0;
}

void sht31Reset()
{
    for (int i = 0; i < 8; i++) cache[i].valid = false;
}

bool sht31Detect(uint8_t addr)
{
    // El SHT31 no tiene un registro de ID simple como otros chips, 
    // pero responderá a un ping o al comando de lectura de status (0xF32D)
    return i2cPing(addr);
}