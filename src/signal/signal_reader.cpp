#include "signal_reader.h"
#include "bus/bus_struct.h"
#include "i2c/pcf_driver.h"

bool leerSignalGPIO(const Signal &s, float &out);
bool leerSignalI2C(const Signal &s, float &out);
bool leerSignalPCF(const Signal &s, float &out);
bool leerSignalModbus(const Signal &s, float &out);

bool leerSignal(const Signal &s, float &out)
{
    bool ok = false;

    switch (s.bus)
    {
    case BusType::BUS_GPIO:
        ok = leerSignalGPIO(s, out);
        break;

    case BusType::BUS_I2C:
        ok = leerSignalI2C(s, out);
        break;

    case BusType::BUS_MODBUS:
        ok = leerSignalModbus(s, out);
        break;

    default:
        return false;
    }

    if (ok && s.invertido && s.kind == SignalKind::SENSOR_DIGITAL)
    {
        out = (out == 0.0f) ? 1.0f : 0.0f;
    }
    return ok;
}
