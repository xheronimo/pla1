#include "sensor_analog.h"

#include "sensor_sources.h"

bool leerSensorAnalogico(const Sensor& s, float& outValue)
{
    float v = 0;

    switch (s.src.type)
    {
        case SRC_GPIO:
            v = analogRead(s.src.channel);
            break;

        case SRC_I2C:
            v = leerI2CAnalogico(s.src.address, s.src.channel);
            break;

        case SRC_MODBUS:
            v = leerModbusAnalogico(s.src.address, s.src.channel);
            break;

        default:
            return false;
    }

    outValue = v;
    return true;
}
