#include "signal_read_pcf85.h"
#include "signal/signal_reader.h"
#include <Wire.h>

static bool pcf8574ReadBit(const Signal& s, float& out)
{
    uint8_t addr = s.address;
    uint8_t bit  = s.channel;

    if (bit > 7)
        return false;

    static bool initialized[128] = {false};

    // Inicializar el PCF una sola vez
    if (!initialized[addr])
    {
        Wire.beginTransmission(addr);
        Wire.write(0xFF);   // todos los pines como entrada
        if (Wire.endTransmission() != 0)
            return false;

        initialized[addr] = true;
    }

    // Leer estado
    Wire.requestFrom(addr, (uint8_t)1);
    if (Wire.available() != 1)
        return false;

    uint8_t value = Wire.read();

    bool level = value & (1 << bit);

    // PCF8574 → activo en LOW normalmente
    out = level ? 1.0f : 0.0f;

    return true;
}
