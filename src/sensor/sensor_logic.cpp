#include "sensor_logic.h"

bool evaluarSensor(
    const Sensor& s,
    SensorRuntime& rt,
    float v)
{
    rt.ultimoValor = v;

    bool nuevoEstado = rt.estado;

    if (s.src.kind == SensorKind::DIGITAL)
    {
        nuevoEstado = (v != 0);
    }
    else
    {
        // ANALÓGICO con histéresis
        if (!rt.estado && v >= rt.hOn)
            nuevoEstado = true;
        else if (rt.estado && v <= rt.hOff)
            nuevoEstado = false;
    }

    // LATCH
    if (s.latch && rt.latched)
        return false;

    if (nuevoEstado && !rt.estado)
    {
        rt.estado = true;
        if (s.latch)
            rt.latched = true;
        return true; // cambio ON
    }

    if (!nuevoEstado && rt.estado)
    {
        rt.estado = false;
        return true; // cambio OFF
    }

    return false;
}
