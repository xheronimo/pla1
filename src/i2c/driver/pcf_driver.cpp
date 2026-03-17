#include "pcf_driver.h"
#include <Wire.h>
#include <string.h>
#include "i2c/i2c_chip_context.h"
#include "i2c/i2c_bus.h"
#include "i2c/i2c_chip_guard.h"
#include "i2c/i2c_chip_registry.h"
#include "system/LogSystem.h"

extern SemaphoreHandle_t semI2C;

// =======================================================
// LÓGICA PRIVADA DE APOYO
// =======================================================

/**
 * @brief Realiza la escritura física en el bus I2C.
 * Maneja automáticamente si es un dispositivo de 8 o 16 bits.
 */
static bool _pcfPhysicalWrite(uint8_t addr, uint16_t value, bool is16Bit) {
    Wire.beginTransmission(addr);
    Wire.write(value & 0xFF);
    if (is16Bit) {
        Wire.write((value >> 8) & 0xFF);
    }
    return (Wire.endTransmission() == 0);
}

// =======================================================
// DETECT & RESET
// =======================================================

bool pcfDetect8574(uint8_t addr) {
    return i2cPing(addr);
}

bool pcfDetect8575(uint8_t addr) {
    // Para el 8575 intentamos una lectura de 2 bytes para confirmar el modelo
    if (!i2cPing(addr)) return false;
    uint8_t buf[2];
    return i2cReadRaw(addr, buf, 2);
}

void pcfReset() {
    size_t count = i2cGetChipCount();
    for (size_t i = 0; i < count; i++) {
        ChipContext* ctx = i2cGetChipContextByIndex(i);
        if (ctx && (ctx->type == I2CDevice::PCF8574 || ctx->type == I2CDevice::PCF8575)) {
            ctx->shadowValid = false; 
            ctx->shadow = 0xFFFF; // Estado de pull-up por defecto
            ctx->state = ChipState::STATE_UNINITIALIZED;
        }
    }
}

// =======================================================
// INIT
// =======================================================

bool pcfInit(uint32_t chipId) {
    ChipContext* cc = i2cGetChipContextById(chipId);
    if (!cc) return false;

    bool is16 = (cc->type == I2CDevice::PCF8575);
    uint16_t safeState = 0xFFFF; // Todas las salidas en OFF / Entradas en Pull-up

    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(100)) == pdTRUE) {
        bool ok = _pcfPhysicalWrite(cc->address, safeState, is16);
        xSemaphoreGive(semI2C);
        
        if (ok) {
            cc->shadow = safeState;
            cc->shadowValid = true;
            cc->state = ChipState::STATE_READY;
            cc->consecutiveErrors = 0;
            return true;
        }
    }
    return false;
}

// =======================================================
// READ SIGNAL
// =======================================================

bool pcfReadSignal(const Signal& s, float& out) {
    ChipContext* cc = i2cGetChipContextById(s.chipId);
    if (!cc || !cc->inUse) return false;

    uint32_t now = millis();
    if (!i2cChipGuardBeforeRead(cc, now)) return false;

    bool ok = false;
    uint16_t value = 0;
    bool is16 = (cc->type == I2CDevice::PCF8575);

    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(50)) == pdTRUE) {
        uint8_t bytesToRead = is16 ? 2 : 1;
        if (Wire.requestFrom(cc->address, bytesToRead) == bytesToRead) {
            value = Wire.read();
            if (is16) value |= (Wire.read() << 8);
            ok = true;
        }
        xSemaphoreGive(semI2C);
    }

    if (ok) {
        cc->shadow = value; // Actualizamos shadow para futuras escrituras
        cc->shadowValid = true;
        out = (value & (1 << s.channel)) ? 1.0f : 0.0f;
        i2cChipGuardOnSuccess(cc, now);
        return true;
    } else {
        i2cChipGuardOnError(cc, ChipMode::MIXED);
        return false;
    }
}

// =======================================================
// WRITE SIGNAL
// =======================================================

bool pcfWriteSignal(const Signal& s, float value) {
    ChipContext* cc = i2cGetChipContextById(s.chipId);
    if (!cc || !cc->inUse) return false;

    uint32_t now = millis();
    if (!i2cChipGuardBeforeRead(cc, now)) return false;

    // Protección crítica: No escribimos si no conocemos el estado de los demás pines
    if (!cc->shadowValid) {
        i2cChipGuardOnError(cc, ChipMode::MIXED);
        return false; 
    }

    uint16_t nextShadow = cc->shadow;
    if (value > 0.5f) nextShadow |= (1 << s.channel);
    else             nextShadow &= ~(1 << s.channel);

    bool ok = false;
    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(50)) == pdTRUE) {
        ok = _pcfPhysicalWrite(cc->address, nextShadow, (cc->type == I2CDevice::PCF8575));
        xSemaphoreGive(semI2C);
    }

    if (ok) {
        cc->shadow = nextShadow;
        i2cChipGuardOnSuccess(cc, now);
        return true;
    } else {
        i2cChipGuardOnError(cc, ChipMode::MIXED);
        return false;
    }
}

// =======================================================
// METADATA
// =======================================================

void pcf8574GetMetadata(ChipMetadata& meta) {
    memset(&meta, 0, sizeof(ChipMetadata));
    strcpy(meta.name, "PCF8574");
    meta.channelCount = 8;
    meta.mode = ChipMode::MIXED;
}

void pcf8575GetMetadata(ChipMetadata& meta) {
    memset(&meta, 0, sizeof(ChipMetadata));
    strcpy(meta.name, "PCF8575");
    meta.channelCount = 16;
    meta.mode = ChipMode::MIXED;
}