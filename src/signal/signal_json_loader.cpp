#include "signal/signal_manager.h"
#include "bus/bus_struct.h"
#include <ArduinoJson.h>
#include <string.h>

// ----------------------------
// PARSERS
// ----------------------------
static SignalKind parseKind(const char *s)
{
    if (s && strcasecmp(s, "analog") == 0)
        return SignalKind::SENSOR_ANALOG;

    return SignalKind::SENSOR_DIGITAL;
}

static BusType parseBus(const char *s)
{
    if (!s)
        return BusType::BUS_GPIO;

    if (strcasecmp(s, "gpio") == 0)
        return BusType::BUS_GPIO;
    if (strcasecmp(s, "pcf") == 0)
        return BusType::BUS_PCF;
    if (strcasecmp(s, "i2c") == 0)
        return BusType::BUS_I2C;
    if (strcasecmp(s, "modbus") == 0)
        return BusType::BUS_MODBUS;

    return BusType::BUS_GPIO;
}

I2CDevice parseI2CDevice(const char *s)
{
    if (!s)
        return I2CDevice::NONE;

    if (!strcasecmp(s, "sht31"))
        return I2CDevice::SHT31;
    

    if (!strcasecmp(s, "pcf8574"))
        return I2CDevice::PCF8574;
    if (!strcasecmp(s, "pcf8575"))
        return I2CDevice::PCF8575;        

    if (!strcasecmp(s, "ads1115"))
        return I2CDevice::ADS1115;
    if (!strcasecmp(s, "bh1750"))
        return I2CDevice::BH1750;

    return I2CDevice::NONE;
}
// ----------------------------
// LOAD SIGNALS
// ----------------------------
bool loadSignalsFromJson(const JsonArrayConst &arr)
{
    for (JsonObjectConst o : arr)
    {
        Signal s{};

        s.id = o["id"] | "";
        s.kind = parseKind(o["kind"] | "digital");
        s.bus = parseBus(o["bus"] | "gpio");
        s.address = o["address"] | 0;
        s.channel = o["channel"] | 0;
        s.invertido = o["invert"] | false;
        s.debounceMs = o["debounce"] | 0;

        // calibration
        if (o.containsKey("calib"))
        {
            JsonObjectConst c = o["calib"];
            s.calib.rawMin = c["rawMin"] | 0.0f;
            s.calib.rawMax = c["rawMax"] | 1.0f;
            s.calib.realMin = c["realMin"] | 0.0f;
            s.calib.realMax = c["realMax"] | 1.0f;
            s.calib.clamp = c["clamp"] | true;
        }

        if (s.bus == BusType::BUS_I2C)
            s.chip = parseI2CDevice(o["chip"]);

        if (!signalManagerAdd(s))
            return false;
    }

    return true;
}
