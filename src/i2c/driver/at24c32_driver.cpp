#include "at24c32_driver.h"
#include "i2c/i2c_chip_guard.h"
#include "i2c/i2c_bus.h"
#include <Arduino.h>
#include <Wire.h>

// ... at24c32Init y at24c32Detect se mantienen igual ...

bool at24c32ReadSignal(const Signal& s, float& out) {
    ChipContext* cc = i2cGetChipContext(I2CDevice::EEPROM, s.address);
    if (!cc) return false;

    uint32_t now = millis();
    if (!i2cChipGuardBeforeRead(cc, now)) return false;

    uint8_t data = 0;
    uint16_t memAddr = (uint16_t)s.channel;

    // Protocolo manual para dirección de 16 bits:
    Wire.beginTransmission(s.address);
    Wire.write((uint8_t)(memAddr >> 8));   // Byte alto
    Wire.write((uint8_t)(memAddr & 0xFF)); // Byte bajo
    if (Wire.endTransmission() != 0) {
        i2cChipGuardOnError(cc);
        return false;
    }

    Wire.requestFrom(s.address, (uint8_t)1);
    if (Wire.available()) {
        data = Wire.read();
    } else {
        i2cChipGuardOnError(cc);
        return false;
    }

    i2cChipGuardOnSuccess(cc, now);
    out = (float)data;
    return true;
}
// ... resto del archivo ...