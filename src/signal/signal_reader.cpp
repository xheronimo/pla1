#include "signal_reader.h"
#include <Arduino.h>

// ---- stubs HW (luego se conectan a drivers reales) ----
bool leerPCFBit(uint8_t addr, uint8_t bit) { return false; }
float leerI2CAnalog(uint8_t addr, uint8_t ch) { return 0.0f; }
bool leerModbusBit(uint8_t slave, uint16_t reg) { return false; }
float leerModbusAnalog(uint8_t slave, uint16_t reg) { return 0.0f; }

// ------------------------------------------------------

bool readSignal(const SignalSourceCfg& cfg, float& out)
{
    float v = 0;

    switch (cfg.source)
    {
        case SignalSource::GPIO:
            v = digitalRead(cfg.pin);
            break;

        case SignalSource::PCF:
            v = leerPCFBit(cfg.addr, cfg.ch);
            break;

        case SignalSource::I2C:
            v = leerI2CAnalog(cfg.addr, cfg.ch);
            break;

        case SignalSource::MODBUS:
            v = (cfg.kind == SignalKind::DIGITAL)
                ? leerModbusBit(cfg.slave, cfg.reg)
                : leerModbusAnalog(cfg.slave, cfg.reg);
            break;

        case SignalSource::ADC:
            v = analogRead(cfg.pin);
            break;
    }

    if (cfg.invert)
        v = !v;

    out = v;
    return true;
}
