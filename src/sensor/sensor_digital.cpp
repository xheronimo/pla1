#include "sensor_digital.h"

#include <Arduino.h>

#include "sensor_sources.h"

bool leerSensorDigital(const Sensor& s, float& outValue)
{
    bool v = false;

    switch (s.src.type)
    {
        case SRC_GPIO:
            v = digitalRead(s.src.channel);
            break;

        case SRC_PCF:
            v = leerPCFBit(s.src.address, s.src.channel);
            break;

        case SRC_I2C:
            v = leerI2CDigital(s.src.address, s.src.channel);
            break;

        case SRC_MODBUS:
            v = leerModbusDigital(s.src.address, s.src.channel);
            break;
    }

    if (s.src.invertido)
        v = !v;

    outValue = v ? 1.0f : 0.0f;
    return true;
}
