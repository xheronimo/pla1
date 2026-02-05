#include "signal_sources.h"
#include "sensor/sensor_manager.h"
#include "signal_struct.h"

bool resolveSignal(SignalDef& sig)
{
    switch (sig.source)
    {
        case SIG_SENSOR:
        {
            // El ID de la señal coincide con el ID del sensor
            float v;
            if (!getSignalValue(String(sig.id), v))
                return false;

            sig.value = v;
            sig.digital = (v != 0);
            return true;
        }

        case SIG_SYSTEM:
            // futuros: reloj, red, watchdog, etc.
            return false;

        case SIG_VIRTUAL:
            // futuro: expresiones, agregados
            return false;
    }

    return false;
}
