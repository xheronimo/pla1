#include "sensor_reader.h"

#include <Arduino.h>

// Stubs (luego se conectan)
bool leerI2CDigital(uint8_t addr, uint8_t bit) { return false; }
bool leerModbusDigital(uint8_t slave, uint16_t reg) { return false; }
bool leerModbusAnalog(uint8_t slave, uint16_t reg, float& v) { v = 0; return true; }

// ==================================================

bool leerSensor(const SensorConfig& s, float& outValue)
{
    switch (s.kind)
    {
        case SensorKind::DIGITAL:
        {
            bool v = false;

            switch (s.src)
            {
                case SensorSourceType::GPIO:
                    v = digitalRead(s.pin);
                    break;

                case SensorSourceType::I2C:
                    v = leerI2CDigital(s.address, s.pin);
                    break;

                case SensorSourceType::MODBUS:
                    v = leerModbusDigital(s.address, s.reg);
                    break;
            }

            if (s.invertido)
                v = !v;

            outValue = v ? 1.0f : 0.0f;
            return true;
        }

        case SensorKind::ANALOG:
        {
            float v = 0;

            if (s.src == SensorSourceType::MODBUS)
            {
                if (!leerModbusAnalog(s.address, s.reg, v))
                    return false;
            }

            outValue = (v * s.escala) + s.offset;
            return true;
        }
    }

    return false;
}
