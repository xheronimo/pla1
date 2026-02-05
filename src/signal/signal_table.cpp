#include "signal/signal_struct.h"

// ⚠️ Ejemplo mínimo — luego vendrá del JSON

Signal signalTable[] = {
    {
        "S1",
        "Entrada digital 1",
        SIG_DIGITAL,
        {
            "S1",
            "GPIO4",
            {
                SRC_GPIO,
                SENSOR_DIGITAL,
                0,
                4,
                false
            },
            0,
            false
        },
        0,
        false
    }
};

uint8_t signalCount = sizeof(signalTable) / sizeof(Signal);
