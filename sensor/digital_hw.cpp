#include "digital_sensor.h"
#include "pcf_manager.h"

bool leerEntradaDigitalHW(const DigitalSensorConfig& cfg)
{
    switch (cfg.tipo)
    {
        case DigitalSensorConfig::GPIO:
            pinMode(cfg.pin, INPUT_PULLUP);
            return digitalRead(cfg.pin);

        case DigitalSensorConfig::PCF:
            return PCF_Read(cfg.chip, cfg.pin);

        case DigitalSensorConfig::MODBUS:
            return false; // se implementará después
    }
    return false;
}
