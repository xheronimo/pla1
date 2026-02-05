#include "sensor_manager.h"

#include "sensor_digital.h"
#include "sensor_analog.h"

bool leerSensor(const Sensor& s, float& outValue)
{
    switch (s.src.kind)
    {
        case SENSOR_DIGITAL:
            return leerSensorDigital(s, outValue);

        case SENSOR_ANALOG:
            return leerSensorAnalogico(s, outValue);
    }
    return false;
}
