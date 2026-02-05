#include "digital_sensor.h"

extern bool leerEntradaDigitalHW(const DigitalSensorConfig& cfg);

DigitalSensor::DigitalSensor(const DigitalSensorConfig& cfg)
: config(cfg)
{
}

void DigitalSensor::update()
{
    bool raw = leerEntrada();

    if (raw != estadoActual)
    {
        uint32_t now = millis();
        if (now - tCambio >= config.histeresisMs)
        {
            estadoPrevio = estadoActual;
            estadoActual = raw;
            tCambio = now;

            if (config.latch && estadoActual)
                latched = true;
        }
    }
    else
    {
        tCambio = millis();
    }
}

bool DigitalSensor::leerEntrada() const
{
    bool v = leerEntradaDigitalHW(config);
    return config.invertido ? !v : v;
}

bool DigitalSensor::estado() const
{
    if (config.latch)
        return latched;
    return estadoActual;
}

bool DigitalSensor::flancoOn() const
{
    return (!estadoPrevio && estadoActual);
}

bool DigitalSensor::flancoOff() const
{
    return (estadoPrevio && !estadoActual);
}

void DigitalSensor::resetLatch()
{
    latched = false;
}
