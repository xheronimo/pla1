#include "alarm_policy.h"
#include <Arduino.h>
#include <math.h>

bool AlarmPolicy::permitirEnvio(
    const AlarmEvent& ev,
    AlarmRuntime& rt,
    uint32_t minIntervalMs)
{
    uint32_t ahora = millis();

    // ⏱️ Anti-spam por tiempo
    if (rt.ultimoEnvioMs &&
        (ahora - rt.ultimoEnvioMs) < minIntervalMs)
        return false;

    // 🔁 Mismo estado y valor casi igual → ignorar
    if (rt.ultimoEstado == ev.estado &&
        fabs(rt.ultimoValor - ev.valor) < 0.01f)
        return false;

    // ✔️ aceptar
    rt.ultimoEnvioMs = ahora;
    rt.ultimoEstado  = ev.estado;
    rt.ultimoValor   = ev.valor;

    return true;
}
