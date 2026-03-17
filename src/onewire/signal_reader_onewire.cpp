#include "signal_reader_onewire.h"
#include "onewire_driver.h"
#include "onewire_metadata.h"
#include <string.h>


bool leerSignalOneWire(const Signal& s, float& out)
{
    const OneWireMeta* meta = onewireGetMeta(s.owRom[0]);
    if (!meta)
        return false;

    if (s.kind == SignalKind::SENSOR_TEMPERATURE && meta->hasTemp)
        return onewireReadTemperature(s.owBus, s.owRom, out);

    if (s.kind == SignalKind::SENSOR_HUMIDITY && meta->hasHum)
        return onewireReadHumidity(s.owBus, s.owRom, out);

    return false;
}