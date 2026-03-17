#include "signal_autogen_onewire.h"
#include "onewire/onewire_bus.h"
#include "onewire/onewire_metadata.h"
#include "signal/signal_manager.h"
#include "board/board_config.h"
#include "onewire_scan.h"

#include <vector>
#include <cstdio>

extern void addSystemSignal(   // ⬅️ IMPORTANTE: NO static
    const char* techId,
    const char* humanName,
    I2CDevice chip,
    uint8_t addr,
    uint8_t chan,
    BusType bus,
    SignalKind kind,
    uint32_t deviceId,
    bool systemReserved,
    bool lockedConfig
);

namespace SignalAutogenOneWire {

void generateAll()
{
    const uint8_t OW_PINS[] = { ONEWIRE_1, ONEWIRE_2, ONEWIRE_3 };

    for (uint8_t pin : OW_PINS) {
        std::vector<OneWireDevice> devs;
        onewireScan(pin, devs);

        for (auto& d : devs) {
            const OneWireMeta* meta = onewireGetMeta(d.family);
            if (!meta) continue;

            char id[32];
            char name[32];

            if (meta->hasTemp) {
                snprintf(id, sizeof(id), "OW_%02X_%02X_TEMP", pin, d.rom[7]);
                snprintf(name, sizeof(name), "%s Temp", meta->name);

                addSystemSignal(
                    id, name,
                    I2CDevice::NONE,
                    pin, 0,
                    BusType::BUS_ONEWIRE,
                    SignalKind::SENSOR_TEMPERATURE,
                    400 + pin,
                    true, true
                );
            }

            if (meta->hasHum) {
                snprintf(id, sizeof(id), "OW_%02X_%02X_HUM", pin, d.rom[7]);
                snprintf(name, sizeof(name), "%s Hum", meta->name);

                addSystemSignal(
                    id, name,
                    I2CDevice::NONE,
                    pin, 1,
                    BusType::BUS_ONEWIRE,
                    SignalKind::SENSOR_HUMIDITY,
                    400 + pin,
                    true, true
                );
            }
        }
    }
}

}