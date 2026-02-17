#include "signal_persist.h"
#include "system/nvs_store.h"
#include "signal_manager.h"
#include <stdio.h>
#include "signal_persist.h"
#include "system/nvs_store.h"
#include "signal/signal_manager.h"

void saveSignalCalibration(const Signal& s)
{
    if (s.kind != SignalKind::SENSOR_ANALOG)
        return;

    auto& nvs = NVS::signals();
    nvs.putBytes(s.id, &s.calib, sizeof(Calibration));
}

void loadSignalCalibrations()
{
    auto& nvs = NVS::signals();

    size_t count;
    Signal* table = signalManagerGetAll(count);

    for (size_t i = 0; i < count; i++)
    {
        Signal& s = table[i];
        if (s.kind != SignalKind::SENSOR_ANALOG)
            continue;

        if (nvs.getBytes(s.id, &s.calib, sizeof(Calibration)) > 0)
        {
            s.initialized = false; // 🔁 recalcular filtros
            s.calib.hasStableValue = false;
            s.calib.emaInit = false;
        }
    }
}
