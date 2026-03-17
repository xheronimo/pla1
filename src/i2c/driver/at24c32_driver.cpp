#include "at24c32_driver.h"
#include "i2c/i2c_chip_guard.h"
#include "i2c/i2c_bus.h"
#include "i2c/i2c_chip_registry.h"
#include <Wire.h>

extern SemaphoreHandle_t semI2C;

// =====================================================
// INIT
// =====================================================
bool at24c32Init(uint8_t addr, uint8_t options) {
    ChipContext* cc = i2cGetChipContext(I2CDevice::EEPROM, addr);
    if (!cc) return false;

    // Verificar presencia física
    if (!i2cPing(addr)) return false;

    cc->state = ChipState::STATE_READY;
    cc->consecutiveErrors = 0;
    cc->errorCount = 0;
    cc->warmupMs = 0; // Memoria instantánea
    cc->retryMs = 5000;

    return true;
}

// =====================================================
// READ SIGNAL (Lectura de 1 Byte)
// =====================================================
bool at24c32ReadSignal(const Signal& s, float& out) {
    // 1. Obtener contexto por chipId
    ChipContext* cc = i2cGetChipContextById(s.chipId);
    if (!cc || !cc->inUse) return false;

    uint32_t now = millis();
    uint8_t addr = cc->address;

    // 2. 🔐 GUARDIA DE SEGURIDAD
    if (!i2cChipGuardBeforeRead(cc, now)) return false;

    bool ok = false;
    uint8_t data = 0;
    uint16_t memAddr = (uint16_t)s.channel; // El canal es la dirección de memoria

    // 3. 🔒 PROTECCIÓN DE BUS
    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(100)) == pdTRUE) {
        
        // Protocolo AT24C32: Dirección de 16 bits (MSB, LSB)
        Wire.beginTransmission(addr);
        Wire.write((uint8_t)(memAddr >> 8));
        Wire.write((uint8_t)(memAddr & 0xFF));
        
        if (Wire.endTransmission() == 0) {
            Wire.requestFrom(addr, (uint8_t)1);
            if (Wire.available()) {
                data = Wire.read();
                ok = true;
            }
        }

        if (ok) {
            i2cChipGuardOnSuccess(cc, now);
        } else {
            i2cChipGuardOnError(cc, ChipMode::INPUT_ONLY);
        }

        xSemaphoreGive(semI2C);
    }

    if (!ok) return false;

    out = (float)data;
    return true;
}

// =====================================================
// WRITE SIGNAL (Escritura de 1 Byte)
// =====================================================
bool at24c32WriteSignal(const Signal& s, float value) {
    ChipContext* cc = i2cGetChipContextById(s.chipId);
    if (!cc || !cc->inUse) return false;

    uint8_t addr = cc->address;
    uint16_t memAddr = (uint16_t)s.channel;
    uint8_t data = (uint8_t)value;
    bool ok = false;

    if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(150)) == pdTRUE) {
        
        Wire.beginTransmission(addr);
        Wire.write((uint8_t)(memAddr >> 8));
        Wire.write((uint8_t)(memAddr & 0xFF));
        Wire.write(data);
        
        if (Wire.endTransmission() == 0) {
            ok = true;
            // ⏳ Ciclo de escritura interno de la EEPROM (5-10ms)
            // Es vital esperar para que el chip vuelva a responder
            delay(10); 
        }

        if (ok) {
            i2cChipGuardOnSuccess(cc, millis());
        } else {
            i2cChipGuardOnError(cc, ChipMode::MIXED);
        }

        xSemaphoreGive(semI2C);
    }

    return ok;
}

// =====================================================
// METADATA
// =====================================================
void at24c32GetMetadata(ChipMetadata& meta) {
    meta.name = "AT24C32";
    meta.channelCount = 4096; // 32Kbits = 4096 Bytes direccionables
    meta.mode = ChipMode::MIXED; 

    meta.opt1 = {"Write Prot", (const char*[]){"OFF", "ON"}, 2, 0};
    meta.opt2 = {"Unused", nullptr, 0, 0};
}

bool at24c32Detect(uint8_t addr) {
    return i2cPing(addr);
}